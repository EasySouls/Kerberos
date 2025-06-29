#pragma once

#include "Kerberos/Core.h"
#include "spdlog/spdlog.h"
#include "spdlog/fmt/ostr.h"

#include <memory>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include "Core/UUID.h"

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

template<>
struct fmt::formatter<glm::vec2> : fmt::formatter<std::string>
{
	auto format(glm::vec2 vec, const format_context& ctx) const -> decltype(ctx.out())
	{
		return fmt::format_to(ctx.out(), "({}, {})", vec.x, vec.y);
	}
};

template<>
struct fmt::formatter<glm::vec3> : fmt::formatter<std::string>
{
	auto format(glm::vec3 vec, const format_context& ctx) const -> decltype(ctx.out())
	{
		return fmt::format_to(ctx.out(), "({}, {}, {})", vec.x, vec.y, vec.z);
	}
};

template<>
struct fmt::formatter<glm::vec4> : fmt::formatter<std::string>
{
	auto format(glm::vec4 vec, const format_context& ctx) const -> decltype(ctx.out())
	{
		return fmt::format_to(ctx.out(), "({}, {}, {}, {})", vec.x, vec.y, vec.z, vec.w);
	}
};

template<>
struct fmt::formatter<Kerberos::UUID> : fmt::formatter<std::string>
{
	auto format(const Kerberos::UUID uuid, const format_context& ctx) const -> decltype(ctx.out())
	{
		return fmt::format_to(ctx.out(), "{}", static_cast<uint64_t>(uuid));
	}
};