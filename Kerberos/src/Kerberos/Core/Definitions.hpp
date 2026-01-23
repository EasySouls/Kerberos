#pragma once

#ifdef _MSC_VER
#define kbr_lifebound [[msvc::lifetimebound]]
#else
#define kbr_lifebound [[clang::lifetimebound]]
#endif
