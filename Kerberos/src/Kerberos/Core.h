#pragma once

#include <memory>

#ifdef KBR_PLATFORM_WINDOWS
#else
	#error Kerberos only supports Windows
#endif

#ifdef KBR_DEBUG
	#define KBR_ENABLE_ASSERTS
#endif

#ifdef KBR_ENABLE_ASSERTS
	#define KBR_ASSERT(x, ...) { if(!(x)) { KBR_ERROR("Assertion Failed: {0}", __VA_ARGS__); __debugbreak(); } }
	#define KBR_CORE_ASSERT(x, ...) { if(!(x)) { KBR_CORE_ERROR("Assertion Failed: {0}", __VA_ARGS__); __debugbreak(); } }
#else
	#define KBR_ASSERT(x, ...)
	#define KBR_CORE_ASSERT(x, ...)
#endif

#define KBR_BIND_EVENT_FN(fn) std::bind(&fn, this, std::placeholders::_1)
#define KBR_BIND_FN(fn) [this]<typename T0>(T0&& PH1) { return fn(std::forward<T0>(PH1)); }

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