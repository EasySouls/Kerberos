#pragma once

#include <memory>

#ifdef KBR_PLATFORM_WINDOWS
#else
	#error Kerberos only supports Windows
#endif

#ifdef KBR_PLATFORM_WINDOWS
	#ifndef NOMINMAX
		#define NOMINMAX
	#endif 
#endif

#ifdef KBR_DEBUG
	#define KBR_ENABLE_ASSERTS
/// For the testing and profiling purposes, we do not need profiling to files.
	//#define KBR_PROFILE

	#if defined(KBR_PLATFORM_WINDOWS)
		#define KBR_DEBUGBREAK() __debugbreak()
	#elif defined(KBR_PLATFORM_LINUX)
		#include <signal.h>
		#define KBR_DEBUGBREAK() raise(SIGTRAP)
	#else
		#error "Platform doesn't support debugbreak yet!"
	#endif
#else
	#define KBR_DEBUGBREAK()
#endif

#ifdef KBR_ENABLE_ASSERTS
#define KBR_ASSERT(x, ...) \
        do { \
            if (!(x)) { \
                KBR_ERROR("Assertion Failed: {0}", __VA_ARGS__); \
                KBR_DEBUGBREAK(); \
            } \
        } while (0)

#define KBR_CORE_ASSERT(x, ...) \
        do { \
            if (!(x)) { \
                KBR_CORE_ERROR("Assertion Failed: {0}", __VA_ARGS__); \
                KBR_DEBUGBREAK(); \
            } \
        } while (0)
#else
	#define KBR_ASSERT(x, ...) do {} while (0)
	#define KBR_CORE_ASSERT(x, ...) do {} while (0)
#endif

#define KBR_VERIFY(x, ...) \
    do { \
        if (!(x)) { \
            KBR_ERROR("Verify Failed: {0}", __VA_ARGS__); \
            __debugbreak(); \
        } \
    } while (0)

#define KBR_CORE_VERIFY(x, ...) \
    do { \
        if (!(x)) { \
            KBR_CORE_ERROR("Verify Failed: {0}", __VA_ARGS__); \
            __debugbreak(); \
        } \
    } while (0)

#define KBR_BIND_EVENT_FN(fn) [this](auto&&... args) -> decltype(auto) { return fn(std::forward<decltype(args)>(args)...); }
#define KBR_BIND_FN(fn) [this]<typename T>(T&& PH1) { return fn(std::forward<T>(PH1)); }

namespace Kerberos
{
	template<typename T>
	using Scope = std::unique_ptr<T>;

	template<typename T, typename ... Args>
	constexpr Scope<T> CreateScope(Args&& ... args)
	{
		return std::make_unique<T>(std::forward<Args>(args)...);
	}

	template<typename T>
	using Ref = std::shared_ptr<T>;

	template<typename T, typename ... Args>
	constexpr Ref<T> CreateRef(Args&& ... args)
	{
		return std::make_shared<T>(std::forward<Args>(args)...);
	}
}