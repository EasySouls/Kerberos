#include "kbrpch.h"
#include "ScriptEngine.h"

#include "Kerberos/Scripting/ScriptInterface.h"
#include "Kerberos/Core/Filesystem.h"

#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/image.h>
#include <mono/metadata/object.h>

#include <unordered_map>



namespace Kerberos
{
	ScriptClass::ScriptClass(MonoImage* image, std::string classNamespace, std::string className)
		: m_ClassNamespace(std::move(classNamespace)), m_ClassName(std::move(className))
	{
		m_MonoClass = mono_class_from_name(image, m_ClassNamespace.c_str(), m_ClassName.c_str());
		KBR_CORE_ASSERT(m_MonoClass, "Failed to find class {0}.{1}", m_ClassNamespace, m_ClassName);
	}

	MonoObject* ScriptClass::Instantiate() const
	{
		return ScriptEngine::InstantiateClass(m_MonoClass);
	}

	MonoMethod* ScriptClass::GetMethod(const std::string& name, const int paramCount) const 
	{
		MonoMethod* method = mono_class_get_method_from_name(m_MonoClass, name.c_str(), paramCount);
		KBR_CORE_ASSERT(method, "Failed to find method {0} in class {1}.{2}", name, m_ClassNamespace, m_ClassName);
		return method;
	}

	MonoObject* ScriptClass::InvokeMethod(MonoMethod* method, MonoObject* instance, void** params) const 
	{
		MonoObject* exception = nullptr;
		MonoObject* result = mono_runtime_invoke(method, instance, params, &exception);
		if (exception)
		{
			MonoString* exceptionMessage = mono_object_to_string(exception, nullptr);
			char* exceptionCStr = mono_string_to_utf8(exceptionMessage);
			std::string exceptionStr(exceptionCStr);
			mono_free(exceptionCStr);
			KBR_CORE_ERROR("Exception thrown when invoking method {0} in class {1}.{2}: {3}", mono_method_get_name(method), m_ClassNamespace, m_ClassName, exceptionStr);
		}
		return result;
	}

	ScriptInstance::ScriptInstance(const Ref<ScriptClass>& scriptClass)
		: m_ScriptClass(scriptClass)
	{
		m_Instance = m_ScriptClass->Instantiate();

		m_OnCreateMethod = m_ScriptClass->GetMethod("OnCreate", 0);
		m_OnUpdateMethod = m_ScriptClass->GetMethod("OnUpdate", 1);
	}


	void ScriptInstance::InvokeOnCreate() const 
	{
		m_ScriptClass->InvokeMethod(m_OnCreateMethod, m_Instance);
	}

	void ScriptInstance::InvokeOnUpdate(float deltaTime) const
	{
		void* params = &deltaTime;
		m_ScriptClass->InvokeMethod(m_OnUpdateMethod, m_Instance, &params);
	}

	struct ScriptEngineData
	{
		MonoDomain* RootDomain = nullptr;
		MonoDomain* AppDomain = nullptr;

		MonoAssembly* CoreAssembly = nullptr;
		MonoImage* CoreAssemblyImage = nullptr;

		ScriptClass EntityClass;

		std::unordered_map<std::string, Ref<ScriptClass>> EntityClasses;
	};

	static ScriptEngineData* s_Data = nullptr;

	static void CppFunc()
	{
		std::cout << "Hello from C++ function called by C#!\n";
	}

	void ScriptEngine::Init()
	{
		s_Data = new ScriptEngineData();

		InitMono();
		LoadAssembly("Resources/Scripts/KerberosScriptCoreLib.dll");
		LoadAssemblyClasses(s_Data->CoreAssembly, s_Data->CoreAssemblyImage);
		auto& classes = s_Data->EntityClasses;

		ScriptInterface::RegisterComponentTypes();
		ScriptInterface::RegisterFunctions();

		mono_add_internal_call("Kerberos.ScriptCoreLib::CppFunc", reinterpret_cast<const void*>(CppFunc));

		s_Data->EntityClass = ScriptClass(s_Data->CoreAssemblyImage, "Kerberos", "ScriptCoreLib");
		MonoObject* instance = s_Data->EntityClass.Instantiate();

		{
			MonoMethod* printCurrentTimeMethod = s_Data->EntityClass.GetMethod("PrintCurrentTime", 0);
			s_Data->EntityClass.InvokeMethod(printCurrentTimeMethod, instance, nullptr);
		}

		{
			MonoMethod* printCustomMessageMethod = s_Data->EntityClass.GetMethod("PrintCustomMessage", 1);
			void* params[1]{
				mono_string_new(s_Data->AppDomain, "Hello from C++!")
			};
			s_Data->EntityClass.InvokeMethod(printCustomMessageMethod, instance, params);
		}
	}

	void ScriptEngine::Shutdown()
	{
		ShutdownMono();

		delete s_Data;
		s_Data = nullptr;
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

		s_Data->RootDomain = rootDomain;
	}

	void ScriptEngine::ShutdownMono() 
	{
		if (s_Data->RootDomain)
		{
			mono_jit_cleanup(s_Data->RootDomain);
			s_Data->RootDomain = nullptr;
		}
	}

	MonoObject* ScriptEngine::InstantiateClass(MonoClass* klass) 
	{
		MonoObject* instance = mono_object_new(s_Data->AppDomain, klass);
		mono_runtime_object_init(instance);

		return instance;
	}

	void ScriptEngine::LoadAssembly(const std::filesystem::path& assemblyPath) 
	{
		s_Data->AppDomain = mono_domain_create_appdomain(const_cast<char*>("KerberosScriptRuntime"), nullptr);
		mono_domain_set(s_Data->AppDomain, true);

		s_Data->CoreAssembly = LoadMonoAssembly(assemblyPath);
		s_Data->CoreAssemblyImage = mono_assembly_get_image(s_Data->CoreAssembly);
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

		MonoClass* entityClass = mono_class_from_name(image, "Kerberos", "Entity");

		for (int32_t i = 0; i < numTypes; i++)
		{
			uint32_t cols[MONO_TYPEDEF_SIZE];
			mono_metadata_decode_row(typeDefinitionsTable, i, cols, MONO_TYPEDEF_SIZE);

			const char* nameSpace = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAMESPACE]);
			const char* name = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAME]);
			const std::string fullname = fmt::format("{}.{}", nameSpace, name);

			KBR_CORE_INFO("Loaded C# class: {}.{}", nameSpace, name);

			if (strcmp(name, "<Module>") == 0)
				continue;

			MonoClass* klass = mono_class_from_name(image, nameSpace, name);

			if (entityClass == klass)
				continue;

			/// If the class is a subclass of Entity, store it in the entities map
			if (mono_class_is_subclass_of(klass, entityClass, false))
			{
				s_Data->EntityClasses[fullname] = CreateRef<ScriptClass>(image, nameSpace, name);
			}

		}
	}

}
