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
