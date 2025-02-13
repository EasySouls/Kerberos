#pragma once

#ifdef KBR_PLATFORM_WINDOWS
	#ifdef KBR_BUILD_DLL
		#define KERBEROS_API __declspec(dllexport)
	#else
		#define KERBEROS_API __declspec(dllimport)
	#endif
#else
	#error Kerberos only supports Windows
#endif

#ifdef KBR_ENABLE_ASSERTS
	#define KBR_ASSERT(x, ...) { if(!(x)) { KBR_ERROR("Assertion Failed: {0}", __VA_ARGS__); __debugbreak(); } }
	#define KBR_CORE_ASSERT(x, ...) { if(!(x)) { KBR_CORE_ERROR("Assertion Failed: {0}", __VA_ARGS__); __debugbreak(); } }
#else
	#define KBR_ASSERT(x, ...)
	#define KBR_CORE_ASSERT(x, ...)
#endif