#pragma once

#include "Kerberos/Core.h"
#include "spdlog/spdlog.h"
#include "spdlog/fmt/ostr.h"

#include <memory>

namespace Kerberos
{
	class Log
	{
	public:
		static void Init();

		inline static Ref<spdlog::logger>& GetCoreLogger() { return s_CoreLogger; }
		inline static Ref<spdlog::logger>& GetClientLogger() { return s_ClientLogger; }

	private:
		static Ref<spdlog::logger> s_CoreLogger;
		static Ref<spdlog::logger> s_ClientLogger;
	};
}

// Core log macros
#define KBR_CORE_TRACE(...)    ::Kerberos::Log::GetCoreLogger()->trace(__VA_ARGS__)
#define KBR_CORE_INFO(...)     ::Kerberos::Log::GetCoreLogger()->info(__VA_ARGS__)
#define KBR_CORE_WARN(...)     ::Kerberos::Log::GetCoreLogger()->warn(__VA_ARGS__)
#define KBR_CORE_ERROR(...)    ::Kerberos::Log::GetCoreLogger()->error(__VA_ARGS__)
#define KBR_CORE_CRITICAL(...) ::Kerberos::Log::GetCoreLogger()->critical(__VA_ARGS__)

// Client log macros
#define KBR_TRACE(...)         ::Kerberos::Log::GetClientLogger()->trace(__VA_ARGS__)
#define KBR_INFO(...)          ::Kerberos::Log::GetClientLogger()->info(__VA_ARGS__)
#define KBR_WARN(...)          ::Kerberos::Log::GetClientLogger()->warn(__VA_ARGS__)
#define KBR_ERROR(...)         ::Kerberos::Log::GetClientLogger()->error(__VA_ARGS__)
#define KBR_CRITICAL(...)      ::Kerberos::Log::GetClientLogger()->critical(__VA_ARGS__)