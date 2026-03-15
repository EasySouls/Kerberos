#include "kbrpch.h"
#include "ScriptClass.h"

#include "ScriptEngine.h"

namespace Kerberos 
{
	ScriptClass::ScriptClass(std::string classNamespace, std::string className, std::string fullName)
		: m_ClassNamespace(std::move(classNamespace)), m_ClassName(std::move(className)), m_FullName(std::move(fullName))
	{
	}
}