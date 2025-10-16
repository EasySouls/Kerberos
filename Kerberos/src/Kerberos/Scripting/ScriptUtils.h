#pragma once

#include "ScriptClass.h"

#include <string_view>

extern "C" {
	typedef struct _MonoType	MonoType;
	typedef struct _MonoString	MonoString;
}

namespace Kerberos
{
	class ScriptUtils
	{
	public:
		static std::string MonoStringToString(MonoString* monoStr);

		static ScriptFieldType MonoTypeToScriptFieldType(MonoType* type);

		static std::string_view ScriptFieldTypeToString(ScriptFieldType type);
		static ScriptFieldType StringToScriptFieldType(std::string_view type);
	};
}