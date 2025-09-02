#include "kbrpch.h"
#include "ScriptInterface.h"

#include "Kerberos/Log.h"

#include <mono/jit/jit.h>

namespace Kerberos
{

#define KBR_ADD_INTERNAL_CALL(name) mono_add_internal_call("Kerberos.ScriptCoreLib::" #name, reinterpret_cast<const void*>(name))

	void ScriptInterface::RegisterComponentTypes()
	{
		
	}

	static void NativeLog(MonoString* message)
	{
		char* messageCStr = mono_string_to_utf8(message);
		std::string messageStr(messageCStr);
		mono_free(messageCStr);
		KBR_CORE_INFO("C# Log: {0}", messageStr);
	}

	void ScriptInterface::RegisterFunctions() 
	{
		KBR_ADD_INTERNAL_CALL(NativeLog);
	}
}
