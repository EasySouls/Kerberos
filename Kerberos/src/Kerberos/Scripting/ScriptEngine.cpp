#include "kbrpch.h"
#include "ScriptEngine.h"

#include "Kerberos/Scripting/ScriptInterface.h"
#include "Kerberos/Scripting/ScriptClass.h"
#include "Kerberos/Scripting/ScriptInstance.h"
#include "Kerberos/Scripting/ScriptUtils.h"
#include "Kerberos/Core/Filesystem.h"
#include "Kerberos/Scene/Scene.h"
#include "Kerberos/Scene/Entity.h"

#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/image.h>
#include <mono/metadata/object.h>
#include <mono/metadata/class.h>
#include <mono/metadata/attrdefs.h>

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

		ScriptClass EntityClass;

		std::unordered_map<std::string, Ref<ScriptClass>> EntityClasses;

		using FieldInitializerMap = std::unordered_map<std::string, ScriptFieldInitializer>;
		/// Holds the data for the initial values of the fields of each entity
		std::unordered_map<UUID, FieldInitializerMap> EntityFieldInitializers;

		/// Runtime data

		std::weak_ptr<Scene> SceneContext;
		std::unordered_map<UUID, Ref<ScriptInstance>> EntityInstances;
	};

	static ScriptEngineData* s_ScriptData = nullptr;

	static void CppFunc()
	{
		std::cout << "Hello from C++ function called by C#!\n";
	}

	void ScriptEngine::Init()
	{
		s_ScriptData = new ScriptEngineData();

		InitMono();
		LoadAssembly("Resources/Scripts/KerberosScriptCoreLib.dll");
		LoadAssemblyClasses(s_ScriptData->CoreAssembly, s_ScriptData->CoreAssemblyImage);

		ScriptInterface::RegisterComponentTypes();
		ScriptInterface::RegisterFunctions();

		mono_add_internal_call("Kerberos.ScriptCoreLib::CppFunc", reinterpret_cast<const void*>(CppFunc));

		s_ScriptData->EntityClass = ScriptClass(s_ScriptData->CoreAssemblyImage, "Kerberos.Source.Kerberos.Scene", "Entity");
		//MonoObject* instance = s_ScriptData->EntityClass.Instantiate();

		//{
		//	MonoMethod* printCurrentTimeMethod = s_ScriptData->EntityClass.GetMethod("PrintCurrentTime", 0);
		//	s_ScriptData->EntityClass.InvokeMethod(printCurrentTimeMethod, instance, nullptr);
		//}

		//{
		//	MonoMethod* printCustomMessageMethod = s_ScriptData->EntityClass.GetMethod("PrintCustomMessage", 1);
		//	void* params[1]{
		//		mono_string_new(s_ScriptData->AppDomain, "Hello from C++!")
		//	};
		//	s_ScriptData->EntityClass.InvokeMethod(printCustomMessageMethod, instance, params);
		//}
	}

	void ScriptEngine::Shutdown()
	{
		ShutdownMono();

		delete s_ScriptData;
		s_ScriptData = nullptr;
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
		if (s_ScriptData->EntityFieldInitializers.contains(entityID))
		{
			/// Initializers already exist for this entity, don't overwrite them
			return;
		}

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

	std::weak_ptr<Scene> ScriptEngine::GetSceneContext()
	{
		return s_ScriptData->SceneContext;
	}

	void ScriptEngine::InitMono() 
	{
		mono_set_assemblies_path("mono/lib");

		MonoDomain* rootDomain = mono_jit_init("KerberosJITRuntime");
		if (rootDomain == nullptr)
		{
			KBR_CORE_ASSERT(rootDomain, "Failed to initialize Mono JIT");
			return;
		}

		s_ScriptData->RootDomain = rootDomain;
	}

	void ScriptEngine::ShutdownMono() 
	{
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

		s_ScriptData->CoreAssembly = LoadMonoAssembly(assemblyPath);
		s_ScriptData->CoreAssemblyImage = mono_assembly_get_image(s_ScriptData->CoreAssembly);
	}

	MonoAssembly* ScriptEngine::LoadMonoAssembly(const std::filesystem::path& assemblyPath) 
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

			KBR_CORE_INFO("Loaded C# class: {}.{}", nameSpace, name);

			if (strcmp(name, "<Module>") == 0)
				continue;

			MonoClass* klass = mono_class_from_name(image, nameSpace, name);

			void* iter = nullptr;
			MonoMethod* method = mono_class_get_methods(klass, &iter);
			while (method != nullptr)
			{
				KBR_CORE_INFO("\t{}", mono_method_get_name(method));
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

					std::cout << "Public field: " << fieldName << " (type: " << typeName << ")\n";

					const ScriptFieldType fieldType = ScriptUtils::MonoTypeToScriptFieldType(type);
					const ScriptField fieldInfo = { .Name = fieldName, .Type = fieldType, .ClassField = field };

					scriptClass->m_SerializedFields[fieldName] = fieldInfo;
					
					/// TODO: Handle default values from C# attributes
				}
			}
		}
	}

	MonoImage* ScriptEngine::GetCoreAssemblyImage() 
	{
		return s_ScriptData->CoreAssemblyImage;
	}
}
