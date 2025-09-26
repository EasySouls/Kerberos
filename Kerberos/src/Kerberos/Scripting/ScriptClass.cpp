#include "kbrpch.h"
#include "ScriptClass.h"

#include "ScriptEngine.h"

#include <mono/metadata/object.h>
#include <mono/metadata/class.h>

namespace Kerberos 
{
	ScriptClass::ScriptClass(MonoImage* image, std::string classNamespace, std::string className)
		: m_ClassNamespace(std::move(classNamespace)), m_ClassName(std::move(className))
	{
		m_MonoClass = mono_class_from_name(image, m_ClassNamespace.c_str(), m_ClassName.c_str());
		KBR_CORE_ASSERT(m_MonoClass, "Failed to find class {0}.{1}", m_ClassNamespace, m_ClassName);
	}

	MonoObject* ScriptClass::Instantiate() const
	{
		return ScriptEngine::InstantiateClass(m_MonoClass);
	}

	MonoMethod* ScriptClass::GetMethod(const std::string& name, const int paramCount) const
	{
		MonoMethod* method = mono_class_get_method_from_name(m_MonoClass, name.c_str(), paramCount);
		KBR_CORE_ASSERT(method, "Failed to find method {0} in class {1}.{2}", name, m_ClassNamespace, m_ClassName);
		return method;
	}

	MonoObject* ScriptClass::InvokeMethod(MonoMethod* method, MonoObject* instance, void** params) const
	{
		MonoObject* exception = nullptr;
		MonoObject* result = mono_runtime_invoke(method, instance, params, &exception);
		if (exception)
		{
			MonoString* exceptionMessage = mono_object_to_string(exception, nullptr);
			char* exceptionCStr = mono_string_to_utf8(exceptionMessage);
			std::string exceptionStr(exceptionCStr);
			mono_free(exceptionCStr);
			KBR_CORE_ERROR("Exception thrown when invoking method {0} in class {1}.{2}: {3}", mono_method_get_name(method), m_ClassNamespace, m_ClassName, exceptionStr);
		}
		return result;
	}
}