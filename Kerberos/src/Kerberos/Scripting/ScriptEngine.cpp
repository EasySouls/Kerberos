#include "kbrpch.h"
#include "ScriptEngine.h"

#include "Kerberos/Scripting/ScriptInterface.h"
#include "Kerberos/Scripting/ScriptClass.h"
#include "Kerberos/Scripting/ScriptInstance.h"
#include "Kerberos/Scripting/ScriptUtils.h"
#include "Kerberos/Core/Filesystem.h"
#include "Kerberos/Scene/Scene.h"
#include "Kerberos/Scene/Entity.h"
#include "Kerberos/Application.h"
#include "Kerberos/Project/Project.h"
#include "Kerberos/Core/Timer.h"

#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/image.h>
#include <mono/metadata/object.h>
#include <mono/metadata/class.h>
#include <mono/metadata/attrdefs.h>
#include <mono/metadata/mono-debug.h>
#include <mono/metadata/threads.h>

#include <filewatch/FileWatch.hpp>

#include <string_view>


using namespace std::literals;

namespace Kerberos
{
	struct ScriptEngineData
	{
		MonoDomain* RootDomain = nullptr;
		MonoDomain* AppDomain = nullptr;

		MonoAssembly* CoreAssembly = nullptr;
		MonoImage* CoreAssemblyImage = nullptr;
		std::filesystem::path CoreAssemblyPath;

		ScriptClass EntityClass;

		std::unordered_map<std::string, Ref<ScriptClass>> EntityClasses;

		using FieldInitializerMap = std::unordered_map<std::string, ScriptFieldInitializer>;
		/// Holds the data for the initial values of the fields of each entity
		std::unordered_map<UUID, FieldInitializerMap> EntityFieldInitializers;

		/// Runtime data

		std::weak_ptr<Scene> SceneContext;
		std::unordered_map<UUID, Ref<ScriptInstance>> EntityInstances;

		bool EnableDebugging = true;
	};

	static ScriptEngineData* s_ScriptData = nullptr;
	static Scope<filewatch::FileWatch<std::string>> s_Filewatcher = nullptr;

	void ScriptEngine::Init()
	{
		s_ScriptData = new ScriptEngineData();

		InitMono();
		LoadAssembly("Resources/Scripts/KerberosScriptCoreLib.dll");
		LoadAssemblyClasses(s_ScriptData->CoreAssembly, s_ScriptData->CoreAssemblyImage);

		ScriptInterface::RegisterComponentTypes();
		ScriptInterface::RegisterFunctions();

		s_ScriptData->EntityClass = ScriptClass(s_ScriptData->CoreAssemblyImage, "Kerberos.Source.Kerberos.Scene", "Entity");

		/// Setup filewatcher to reload assembly on changes
		/// TODO: Use the relative path from the project directory

		/// TODO: FIX Project is not initialized at this point :C
		//const std::filesystem::path scriptDir = Project::GetAssetDirectory () / ".."sv / "Resources"sv / "Scripts"sv;

		const std::filesystem::path assemblyPath = std::filesystem::current_path() / "Resources"sv / "Scripts"sv;
		s_Filewatcher = CreateScope<filewatch::FileWatch<std::string>>(
			assemblyPath.string(),
			OnAssemblyFileChanged
		);
	}

	void ScriptEngine::Shutdown()
	{
		ShutdownMono();

		delete s_ScriptData;
		s_ScriptData = nullptr;
	}

	void ScriptEngine::ReloadAssembly()
	{
		Timer reloadAssemblyTimer("Reload Assembly",[&](const TimerData& data)
		{
			KBR_CORE_INFO("Reloading C# assemblies took {:.2f} ms", data.DurationMs);
		});

		mono_domain_set(mono_get_root_domain(), false);
		mono_domain_unload(s_ScriptData->AppDomain);
		s_ScriptData->AppDomain = nullptr;

		LoadAssembly(s_ScriptData->CoreAssemblyPath);
		LoadAssemblyClasses(s_ScriptData->CoreAssembly, s_ScriptData->CoreAssemblyImage);

		/// The registered classes use the MonoImage, so they have to be reloaded with the new image
		ScriptInterface::RegisterComponentTypes();
	}

	void ScriptEngine::OnRuntimeStart(const Ref<Scene>& scene)
	{
		s_ScriptData->SceneContext = scene;		
	}

	void ScriptEngine::OnRuntimeStop() 
	{
		s_ScriptData->SceneContext.reset();
		s_ScriptData->EntityInstances.clear();
	}

	void ScriptEngine::OnCreateEntity(const Entity entity) 
	{
		auto& scriptComponent = entity.GetComponent<ScriptComponent>();
		
		if (!ClassExists(scriptComponent.ClassName))
		{
			KBR_CORE_ERROR("Script class '{0}' does not exist!", scriptComponent.ClassName);
			return;
		}

		Ref<ScriptInstance> instance = nullptr;

		/// Apply initial field values when instantiating the script, if there are any
		if (const auto& fieldInitializers = GetScriptFieldInitializerMap(entity); !fieldInitializers.empty())
		{
			instance = CreateRef<ScriptInstance>(s_ScriptData->EntityClasses[scriptComponent.ClassName], entity, fieldInitializers);
		}
		else
		{
			instance = CreateRef<ScriptInstance>(s_ScriptData->EntityClasses[scriptComponent.ClassName], entity, std::unordered_map<std::string, ScriptFieldInitializer>());
		}

		const UUID entityID = entity.GetUUID();
		s_ScriptData->EntityInstances[entityID] = instance;

		instance->InvokeOnCreate();
	}

	void ScriptEngine::OnUpdateEntity(const Entity entity, const float deltaTime)
	{
		KBR_CORE_ASSERT(entity.HasComponent<ScriptComponent>(), "Entity does not have a ScriptComponent!");
		KBR_CORE_ASSERT(s_ScriptData->EntityInstances.contains(entity.GetUUID()), "No script instance found for entity!");

		s_ScriptData->EntityInstances[entity.GetUUID()]->InvokeOnUpdate(deltaTime);
	}

	bool ScriptEngine::ClassExists(const std::string& className) 
	{
		return s_ScriptData->EntityClasses.contains(className);
	}

	void ScriptEngine::CreateScriptFieldInitializers(const Entity entity, const std::string& className) 
	{
		KBR_CORE_ASSERT(ClassExists(className), "Script class doesn't exist!");

		const UUID entityID = entity.GetUUID();
		const std::string_view currentClassName = entity.GetComponent<ScriptComponent>().ClassName;
		if (s_ScriptData->EntityFieldInitializers.contains(entityID) && currentClassName == className)
		{
			/// Initializers already exist for this entity, don't overwrite them
			return;
		}

		/// TODO: if the entity has a script class and the name is the same, but the fields are not, it doesn't update the initializers

		const Ref<ScriptClass>& scriptClass = s_ScriptData->EntityClasses.at(className);
		const auto& serializedFields = scriptClass->GetSerializedFields();

		ScriptEngineData::FieldInitializerMap fieldInitializers;
		for (const auto& [name, field] : serializedFields)
		{
			ScriptFieldInitializer fieldInitializer;
			fieldInitializer.Field = field;

			fieldInitializers[name] = fieldInitializer;
		}

		s_ScriptData->EntityFieldInitializers[entityID] = fieldInitializers;
	}

	const std::unordered_map<std::string, Ref<ScriptClass>>& ScriptEngine::GetEntityClasses() 
	{
		return s_ScriptData->EntityClasses;
	}

	std::unordered_map<std::string, ScriptFieldInitializer>& ScriptEngine::GetScriptFieldInitializerMap(const Entity entity)
	{
		const UUID entityID = entity.GetUUID();
		if (!s_ScriptData->EntityFieldInitializers.contains(entityID))
		{
			const std::string_view entityName = entity.GetName();
			KBR_CORE_TRACE("No field initializers found for entity {}"sv, entityName);

			/// Create an empty map for this entity if it doesn't exist
			return s_ScriptData->EntityFieldInitializers[entityID];
		}

		return s_ScriptData->EntityFieldInitializers.at(entityID);
	}

	Ref<ScriptInstance> ScriptEngine::GetEntityInstance(const UUID entityID)
	{
		if (!s_ScriptData->EntityInstances.contains(entityID))
		{
			/// We return nullptr instaed of asserting, since this can be called when the game isn't running,
			/// thus no instance exist
			
			/// TODO: Design a more robust system, where the user can still see and set the available fields of the script in the editor,
			/// and those are applied when the game starts.
			return nullptr;
			//KBR_CORE_ASSERT(s_ScriptData->EntityInstances.contains(entityID), "No script instance found for entity!");
		}

		return s_ScriptData->EntityInstances.at(entityID);
	}

	const std::weak_ptr<Scene>& ScriptEngine::GetSceneContext()
	{
		return s_ScriptData->SceneContext;
	}

	void ScriptEngine::InitMono() 
	{
		mono_set_assemblies_path("mono/lib");

		if (s_ScriptData->EnableDebugging)
		{
			const char* argv[2] = {
				"--debugger-agent=transport=dt_socket,address=127.0.0.1:2550,server=y,suspend=n,loglevel=3,logfile=MonoDebugger.log",
				"--soft-breakpoints"
			};

			mono_jit_parse_options(2, const_cast<char**>(argv));
			mono_debug_init(MONO_DEBUG_FORMAT_MONO);
		}

		MonoDomain* rootDomain = mono_jit_init("KerberosJITRuntime");
		if (rootDomain == nullptr)
		{
			KBR_CORE_ASSERT(rootDomain, "Failed to initialize Mono JIT");
			return;
		}

		s_ScriptData->RootDomain = rootDomain;

		if (s_ScriptData->EnableDebugging)
			mono_debug_domain_create(s_ScriptData->RootDomain);

		mono_thread_set_main(mono_thread_current());
	}

	void ScriptEngine::ShutdownMono() 
	{
		if (s_ScriptData->AppDomain)
		{
			mono_domain_set(mono_get_root_domain(), false);
			mono_domain_unload(s_ScriptData->AppDomain);
			s_ScriptData->AppDomain = nullptr;
		}

		if (s_ScriptData->RootDomain)
		{
			mono_jit_cleanup(s_ScriptData->RootDomain);
			s_ScriptData->RootDomain = nullptr;
		}
	}

	MonoObject* ScriptEngine::InstantiateClass(MonoClass* klass) 
	{
		MonoObject* instance = mono_object_new(s_ScriptData->AppDomain, klass);
		mono_runtime_object_init(instance);

		return instance;
	}

	void ScriptEngine::LoadAssembly(const std::filesystem::path& assemblyPath) 
	{
		s_ScriptData->AppDomain = mono_domain_create_appdomain(const_cast<char*>("KerberosScriptRuntime"), nullptr);
		mono_domain_set(s_ScriptData->AppDomain, true);

		s_ScriptData->CoreAssembly = LoadMonoAssembly(assemblyPath, s_ScriptData->EnableDebugging);
		s_ScriptData->CoreAssemblyImage = mono_assembly_get_image(s_ScriptData->CoreAssembly);
		s_ScriptData->CoreAssemblyPath = assemblyPath;
	}

	MonoAssembly* ScriptEngine::LoadMonoAssembly(const std::filesystem::path& assemblyPath, bool loadPdb) 
	{
		uint32_t fileSize = 0;
		char* fileData = Filesystem::ReadBytes(assemblyPath, &fileSize);

		/// NOTE: We can't use this image for anything other than loading the assembly because this image doesn't have a reference to the assembly
		MonoImageOpenStatus status;
		MonoImage* image = mono_image_open_from_data_full(fileData, fileSize, 1, &status, 0);

		if (status != MONO_IMAGE_OK)
		{
			const char* errorMessage = mono_image_strerror(status);
			KBR_CORE_ASSERT(false, "Failed to load assembly from file {0}: {1}", assemblyPath, errorMessage);
			return nullptr;
		}

		if (loadPdb)
		{
			std::filesystem::path pdbPath = assemblyPath;

			pdbPath.replace_extension(".pdb");

			if (std::filesystem::exists(pdbPath))
			{
				uint32_t pdbFileSize = 0;
				const char* pdbFileData = Filesystem::ReadBytes(pdbPath, &pdbFileSize);
				mono_debug_open_image_from_memory(image, reinterpret_cast<const mono_byte*>(pdbFileData), static_cast<int>(pdbFileSize));
				KBR_CORE_INFO("Loaded PDB {}", pdbPath);
				delete[] pdbFileData;
			}
		}

		MonoAssembly* assembly = mono_assembly_load_from_full(image, assemblyPath.string().c_str(), &status, 0);
		mono_image_close(image);

		delete[] fileData;

		return assembly;
	}

	void ScriptEngine::LoadAssemblyClasses(const MonoAssembly* assembly, MonoImage* image)
	{
		KBR_CORE_ASSERT(assembly, "Assembly is null!");
		KBR_CORE_ASSERT(image, "Image is null!");

		const MonoTableInfo* typeDefinitionsTable = mono_image_get_table_info(image, MONO_TABLE_TYPEDEF);
		const int32_t numTypes = mono_table_info_get_rows(typeDefinitionsTable);

		MonoClass* entityClass = mono_class_from_name(image, "Kerberos.Source.Kerberos.Scene", "Entity");

		for (int32_t i = 0; i < numTypes; i++)
		{
			uint32_t cols[MONO_TYPEDEF_SIZE];
			mono_metadata_decode_row(typeDefinitionsTable, (int)i, cols, MONO_TYPEDEF_SIZE);

			const char* nameSpace = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAMESPACE]);
			const char* name = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAME]);

			const std::string fullname = fmt::format("{}.{}", nameSpace, name);

			KBR_CORE_TRACE("Loaded C# class: {}.{}", nameSpace, name);

			if (strcmp(name, "<Module>") == 0)
				continue;

			MonoClass* klass = mono_class_from_name(image, nameSpace, name);

			void* iter = nullptr;
			MonoMethod* method = mono_class_get_methods(klass, &iter);
			while (method != nullptr)
			{
				KBR_CORE_TRACE("\t{}", mono_method_get_name(method));
				method = mono_class_get_methods(klass, &iter);
			}

			if (entityClass == klass)
				continue;

			if (!mono_class_is_subclass_of(klass, entityClass, false))
				continue;

			/// If the class is a subclass of Entity, store it in the entities map
			const Ref<ScriptClass> scriptClass = CreateRef<ScriptClass>(image, nameSpace, name);
			s_ScriptData->EntityClasses[fullname] = scriptClass;

			void* fieldIterator = nullptr;
			MonoClassField* field;
			while ((field = mono_class_get_fields(klass, &fieldIterator)) != nullptr)
			{
				const uint32_t flags = mono_field_get_flags(field);
				if (flags == MONO_FIELD_ATTR_PUBLIC)
				{
					/// TODO: Filter by custom C# attribute [ShowInEditor]

					const std::string fieldName = mono_field_get_name(field);
					MonoType* type = mono_field_get_type(field);
					const char* typeName = mono_type_get_name(type);

					KBR_CORE_TRACE("\n\tPublic field: {0}, type: {1}", fieldName, typeName);

					const ScriptFieldType fieldType = ScriptUtils::MonoTypeToScriptFieldType(type);
					const ScriptField fieldInfo = { .Name = fieldName, .Type = fieldType, .ClassField = field };

					scriptClass->m_SerializedFields[fieldName] = fieldInfo;
					
					/// TODO: Handle default values from C# attributes
				}
			}
		}
	}

	static std::string_view FileWatchEventToString(const filewatch::Event event)
	{
		switch (event)
		{
		case filewatch::Event::added: return "Added"sv;
		case filewatch::Event::removed: return "Removed"sv;
		case filewatch::Event::modified: return "Modified"sv;
		case filewatch::Event::renamed_old: return "Renamed Old"sv;
		case filewatch::Event::renamed_new: return "Renamed New"sv;
		}

		KBR_CORE_ASSERT(false, "Unknown filewatch::Event");
		return "Unknown"sv;
	}

	void ScriptEngine::OnAssemblyFileChanged(const std::string& path, const filewatch::Event changeType) 
	{
		if (changeType == filewatch::Event::modified)
		{
			const std::string extension = std::filesystem::path(path).extension().string();
			if (extension == ".dll") {
				KBR_CORE_INFO("Assembly file modified: {0}", path);
				Application::Get().SubmitToMainThread([](){
					ReloadAssembly();
				});
			}
		}
	}

	MonoImage* ScriptEngine::GetCoreAssemblyImage() 
	{
		return s_ScriptData->CoreAssemblyImage;
	}
}
