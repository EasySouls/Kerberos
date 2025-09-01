#pragma once

typedef struct _MonoAssembly MonoAssembly;

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

		static MonoAssembly* LoadCSharpAssembly(const std::string& assemblyPath);
		static void PrintAssemblyTypes(MonoAssembly* assembly);
		static char* ReadBytes(const std::string& filepath, uint32_t* outSize);
	};
}
