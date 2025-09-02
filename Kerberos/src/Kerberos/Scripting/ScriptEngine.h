#pragma once

#include <filesystem>

extern "C" {
	typedef struct _MonoAssembly	MonoAssembly;
	typedef struct _MonoClass		MonoClass;
	typedef struct _MonoObject		MonoObject;
	typedef struct _MonoMethod		MonoMethod;
}

namespace Kerberos
{
	class ScriptEngine
	{
	public:
		static void Init();
		static void Shutdown();

	private:
		static void InitMono();
		static void ShutdownMono();

		static void LoadAssembly(const std::filesystem::path& assemblyPath);
		static MonoAssembly* LoadMonoAssembly(const std::filesystem::path& assemblyPath);
		static void PrintAssemblyTypes(MonoAssembly* assembly);
	};
}
