#include "kbrpch.h"
#include "ScriptEngine.h"

#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/image.h>
#include <mono/metadata/object.h>

#include <print>


namespace Kerberos
{
	struct ScriptEngineData
	{
		MonoDomain* RootDomain = nullptr;
		MonoDomain* AppDomain = nullptr;

		MonoAssembly* CoreAssembly = nullptr;
	};

	static ScriptEngineData* s_Data = nullptr;

	void ScriptEngine::Init()
	{
		s_Data = new ScriptEngineData();
		InitMono();
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

		s_Data->AppDomain = mono_domain_create_appdomain(const_cast<char*>("KerberosScriptRuntime"), nullptr);
		mono_domain_set(s_Data->AppDomain, true);

		s_Data->CoreAssembly = LoadCSharpAssembly("Resources/Scripts/KerberosScriptCoreLib.dll");
		PrintAssemblyTypes(s_Data->CoreAssembly);


		MonoImage* image = mono_assembly_get_image(s_Data->CoreAssembly);
		MonoClass* klass = mono_class_from_name(image, "Kerberos", "ScriptCoreLib");
		MonoObject* obj = mono_object_new(s_Data->AppDomain, klass);
		mono_runtime_object_init(obj);

		MonoMethod* printCurrentTimeMethod = mono_class_get_method_from_name(klass, "PrintCurrentTime", 0);
		mono_runtime_invoke(printCurrentTimeMethod, obj, nullptr, nullptr);
		
		{
			MonoMethod* printCustomMessageMethod = mono_class_get_method_from_name(klass, "PrintCustomMessage", 1);
			void* params[1] {
				mono_string_new(s_Data->AppDomain, "Hello from C++!")
			};
			mono_runtime_invoke(printCustomMessageMethod, obj, params, nullptr);
		}
	}

	void ScriptEngine::ShutdownMono() 
	{
		if (s_Data->RootDomain)
		{
			mono_jit_cleanup(s_Data->RootDomain);
			s_Data->RootDomain = nullptr;
		}
	}

	MonoAssembly* ScriptEngine::LoadCSharpAssembly(const std::string& assemblyPath)
	{
		uint32_t fileSize = 0;
		char* fileData = ReadBytes(assemblyPath, &fileSize);

		/// NOTE: We can't use this image for anything other than loading the assembly because this image doesn't have a reference to the assembly
		MonoImageOpenStatus status;
		MonoImage* image = mono_image_open_from_data_full(fileData, fileSize, 1, &status, 0);

		if (status != MONO_IMAGE_OK)
		{
			const char* errorMessage = mono_image_strerror(status);
			KBR_CORE_ASSERT(false, "Failed to load assembly from file {0}: {1}", assemblyPath, errorMessage);
			return nullptr;
		}

		MonoAssembly* assembly = mono_assembly_load_from_full(image, assemblyPath.c_str(), &status, 0);
		mono_image_close(image);

		delete[] fileData;

		return assembly;
	}

	void ScriptEngine::PrintAssemblyTypes(MonoAssembly* assembly)
	{
		MonoImage* image = mono_assembly_get_image(assembly);
		const MonoTableInfo* typeDefinitionsTable = mono_image_get_table_info(image, MONO_TABLE_TYPEDEF);
		const int32_t numTypes = mono_table_info_get_rows(typeDefinitionsTable);

		for (int32_t i = 0; i < numTypes; i++)
		{
			uint32_t cols[MONO_TYPEDEF_SIZE];
			mono_metadata_decode_row(typeDefinitionsTable, i, cols, MONO_TYPEDEF_SIZE);

			const char* nameSpace = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAMESPACE]);
			const char* name = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAME]);

			std::println("{}.{}", nameSpace, name);
		}
	}

	char* ScriptEngine::ReadBytes(const std::string& filepath, uint32_t* outSize)
	{
		std::ifstream stream(filepath, std::ios::binary | std::ios::ate);

		if (!stream)
		{
			return nullptr;
		}

		const std::streampos end = stream.tellg();
		stream.seekg(0, std::ios::beg);
		const std::streamoff size = end - stream.tellg();

		if (size == 0)
		{
			return nullptr;
		}

		char* buffer = new char[size];
		stream.read(buffer, size);
		stream.close();

		*outSize = size;
		return buffer;
	}

}
