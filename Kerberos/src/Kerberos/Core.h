#pragma once

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