#pragma once

#include <string>

extern "C" {
	typedef struct _MonoClass		MonoClass;
	typedef struct _MonoObject		MonoObject;
	typedef struct _MonoMethod		MonoMethod;
	typedef struct _MonoImage		MonoImage;
	typedef struct _MonoClassField	MonoClassField;
}

namespace Kerberos { class ScriptEngine;	}
namespace Kerberos { class ScriptInstance;	}

namespace Kerberos 
{

	/// Usable field types in the editor for script components
	enum class ScriptFieldType
	{
		Short,
		Int,
		Long,
		UShort,
		UInt,
		ULong,
		Float,
		Double,
		Bool, 
		Char,
		Byte,
		String,

		Vec2,
		Vec3, 
		Vec4,

		AssetHandle
	};

	struct ScriptField
	{
		std::string Name;
		ScriptFieldType Type;
		MonoClassField* ClassField;
	};

	/*
	* Represent a C# class in the scripting system.
	*/
	class ScriptClass
	{
	public:
		ScriptClass() = default;
		ScriptClass(MonoImage* image, std::string classNamespace, std::string className);

		MonoObject* Instantiate() const;

		MonoMethod* GetMethod(const std::string& name, int paramCount) const;
		MonoObject* InvokeMethod(MonoMethod* method, MonoObject* instance, void** params = nullptr) const;

		const std::unordered_map<std::string, ScriptField>& GetSerializedFields() const { return m_SerializedFields; }

	private:
		std::string m_ClassNamespace;
		std::string m_ClassName;

		/// Fields that should be serialized and visible in the editor
		/// This consists of public fields which do not have the [SerializeField(false)] attribute,
		/// and private fields which have the [SerializeField(true)] attribute.
		std::unordered_map<std::string, ScriptField> m_SerializedFields;

		MonoClass* m_MonoClass = nullptr;

		friend class ScriptEngine;
		friend class ScriptInstance;
	};
}