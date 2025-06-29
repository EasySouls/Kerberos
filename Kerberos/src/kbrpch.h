#pragma once

#ifdef KBR_PLATFORM_WINDOWS
	#include <Windows.h>
#endif

#include <iostream>
#include <memory>
#include <utility>
#include <algorithm>
#include <functional>

#include <string>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <set>

#include "Kerberos/Core/Buffer.h"
#include "Kerberos/Core.h"
#include "Kerberos/Log.h"
#include "Kerberos/Debug/Instrumentor.h"