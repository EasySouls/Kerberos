#include "kbrpch.h"
#include "ScriptUtils.h"

#include "ScriptClass.h"

//#include <mono/metadata/metadata.h>
//#include <mono/metadata/blob.h>
#include <mono/metadata/class.h>
#include <mono/metadata/object.h>

#include <unordered_map>

namespace Kerberos
{
	std::string ScriptUtils::MonoStringToString(MonoString* monoStr)
	{
		if (monoStr == nullptr)
			return std::string();

		char* cStr = mono_string_to_utf8(monoStr);
		std::string str(cStr);

		mono_free(cStr);
		return str;
	}

	static std::unordered_map<std::string, ScriptFieldType> s_ScriptFieldTypeMap = {
		{ "System.Int16",								ScriptFieldType::Short },
		{ "System.Int32",								ScriptFieldType::Int },
		{ "System.Int64",								ScriptFieldType::Long },
		{ "System.UInt16",								ScriptFieldType::UShort },
		{ "System.UInt32",								ScriptFieldType::UInt },
		{ "System.UInt64",								ScriptFieldType::ULong },
		{ "System.Single",								ScriptFieldType::Float },
		{ "System.Double",								ScriptFieldType::Double },
		{ "System.Boolean",								ScriptFieldType::Bool },
		{ "System.Char",								ScriptFieldType::Char },
		{ "System.Byte",								ScriptFieldType::Byte },
		{ "System.String",								ScriptFieldType::String },

		{ "Kerberos.Source.Kerberos.Core.Vector2",		ScriptFieldType::Vec2 },
		{ "Kerberos.Source.Kerberos.Core.Vector3",		ScriptFieldType::Vec3 },
		{ "Kerberos.Source.Kerberos.Core.Vector4",		ScriptFieldType::Vec4 },

		{ "Kerberos.Source.Kerberos.Core.AssetHandle",	ScriptFieldType::AssetHandle }
	};

	ScriptFieldType ScriptUtils::MonoTypeToScriptFieldType(MonoType* type)
	{
		KBR_CORE_ASSERT(type, "MonoType is null!");

		std::string typeName = mono_type_get_name(type);

		KBR_CORE_ASSERT(s_ScriptFieldTypeMap.contains(typeName), "Unsupported field type: {0}", typeName);

		return s_ScriptFieldTypeMap.at(typeName);
	}

	std::string_view ScriptUtils::ScriptFieldTypeToString(const ScriptFieldType type)
	{
		switch (type)
		{
		case ScriptFieldType::Short:		return "Short";
		case ScriptFieldType::Int:			return "Int";
		case ScriptFieldType::Long:			return "Long";
		case ScriptFieldType::UShort:		return "UShort";
		case ScriptFieldType::UInt:			return "UInt";
		case ScriptFieldType::ULong:		return "ULong";
		case ScriptFieldType::Float:		return "Float";
		case ScriptFieldType::Double:		return "Double";
		case ScriptFieldType::Bool:			return "Bool";
		case ScriptFieldType::Char:			return "Char";
		case ScriptFieldType::Byte:			return "Byte";
		case ScriptFieldType::String:		return "String";
		case ScriptFieldType::Vec2:			return "Vector2";
		case ScriptFieldType::Vec3:			return "Vector3";
		case ScriptFieldType::Vec4:			return "Vector4";
		case ScriptFieldType::AssetHandle:	return "AssetHandle";
		}

		KBR_CORE_ASSERT(false, "Unknown script field type!");
		return "";
	}

	ScriptFieldType ScriptUtils::StringToScriptFieldType(const std::string_view type) 
	{
		if (type == "Short")			return ScriptFieldType::Short;
		if (type == "Int")				return ScriptFieldType::Int;
		if (type == "Long")				return ScriptFieldType::Long;
		if (type == "UShort")			return ScriptFieldType::UShort;
		if (type == "UInt")				return ScriptFieldType::UInt;
		if (type == "ULong")			return ScriptFieldType::ULong;
		if (type == "Float")			return ScriptFieldType::Float;
		if (type == "Double")			return ScriptFieldType::Double;
		if (type == "Bool")				return ScriptFieldType::Bool;
		if (type == "Char")				return ScriptFieldType::Char;
		if (type == "Byte")				return ScriptFieldType::Byte;
		if (type == "String")			return ScriptFieldType::String;
		if (type == "Vector2")			return ScriptFieldType::Vec2;
		if (type == "Vector3")			return ScriptFieldType::Vec3;
		if (type == "Vector4")			return ScriptFieldType::Vec4;
		if (type == "AssetHandle")		return ScriptFieldType::AssetHandle;

		KBR_CORE_ASSERT(false, "Unknown script field type!");
		return ScriptFieldType::Int;
	}


	/*ScriptFieldType ScriptUtils::MonoTypeToScriptFieldType(MonoType* type)
	{
		KBR_CORE_ASSERT(type, "MonoType is null!");

		int val = mono_type_get_type(type);
		const MonoTypeEnum typeEnum = static_cast<MonoTypeEnum>(val);
		switch (typeEnum)
		{
		case MONO_TYPE_I4:		return ScriptFieldType::Int;
		case MONO_TYPE_R4:		return ScriptFieldType::Float;
		case MONO_TYPE_BOOLEAN: return ScriptFieldType::Bool;
		case MONO_TYPE_STRING:	return ScriptFieldType::String;
		case MONO_TYPE_VALUETYPE:
		{
			MonoClass* klass = mono_type_get_class(type);
			const char* name = mono_class_get_name(klass);

			KBR_CORE_WARN("Value type name: {0}", name);

			if (strcmp(name, "Vector2") == 0)
				return ScriptFieldType::Vec2;
			if (strcmp(name, "Vector3") == 0)
				return ScriptFieldType::Vec3;
			if (strcmp(name, "Vector4") == 0)
				return ScriptFieldType::Vec4;
		}
		}
	}*/
}
