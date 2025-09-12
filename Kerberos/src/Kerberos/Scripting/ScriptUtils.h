#pragma once

#include "ScriptClass.h"

extern "C" {
	typedef struct _MonoType	MonoType;
}

namespace Kerberos
{
	class ScriptUtils
	{
	public:
		static ScriptFieldType MonoTypeToScriptFieldType(MonoType* type);
	};
}