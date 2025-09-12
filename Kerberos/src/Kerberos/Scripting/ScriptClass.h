#pragma once

#include <string>

extern "C" {
	typedef struct _MonoClass		MonoClass;
	typedef struct _MonoObject		MonoObject;
	typedef struct _MonoMethod		MonoMethod;
	typedef struct _MonoImage		MonoImage;
}

namespace Kerberos 
{

	/// Usable field types in the editor for script components
	enum class ScriptFieldType
	{
		Int, 
		Float,
		Double,
		Bool, 
		String,
		Vec2,
		Vec3, 
		Vec4,
		AssetHandle
	};

	class ScriptClass
	{
	public:
		ScriptClass() = default;
		ScriptClass(MonoImage* image, std::string classNamespace, std::string className);

		MonoObject* Instantiate() const;

		MonoMethod* GetMethod(const std::string& name, int paramCount) const;
		MonoObject* InvokeMethod(MonoMethod* method, MonoObject* instance, void** params = nullptr) const;

	private:
		std::string m_ClassNamespace;
		std::string m_ClassName;

		MonoClass* m_MonoClass = nullptr;
	};
}