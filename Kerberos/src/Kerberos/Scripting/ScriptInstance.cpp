#include "kbrpch.h"
#include "ScriptInstance.h"

#include "ScriptClass.h"

#include <mono/metadata/object.h>

namespace Kerberos
{
	ScriptInstance::ScriptInstance(const Ref<ScriptClass>& scriptClass, const Entity entity, const std::unordered_map<std::string, ScriptFieldInitializer>& initialFieldValues)
		: m_Entity(entity), m_ScriptClass(scriptClass)
	{
		m_Instance = m_ScriptClass->Instantiate();

		KBR_CORE_ASSERT(m_Instance, "Failed to instantiate ScriptInstance!");

		m_OnCreateMethod = m_ScriptClass->GetMethod("OnCreate", 0);
		m_OnUpdateMethod = m_ScriptClass->GetMethod("OnUpdate", 1);
		m_Constructor = m_ScriptClass->GetMethod(".ctor", 1);

		UUID entityID = m_Entity.GetUUID();
		void* params[1] = { &entityID };
		m_ScriptClass->InvokeMethod(m_Constructor, m_Instance, params);

		/// Set initial field values after calling the constructor
		InitializeValues(initialFieldValues);
	}


	void ScriptInstance::InvokeOnCreate() const
	{
		/// The OnCreate method is not necesserily overrided
		if (m_OnCreateMethod)
			m_ScriptClass->InvokeMethod(m_OnCreateMethod, m_Instance);
	}

	void ScriptInstance::InvokeOnUpdate(float deltaTime) const
	{
		/// The OnUpdate method is not necesserily overrided
		if (m_OnUpdateMethod)
		{
			void* params = &deltaTime;
			m_ScriptClass->InvokeMethod(m_OnUpdateMethod, m_Instance, &params);
		}
	}

	void ScriptInstance::InitializeValues(const std::unordered_map<std::string, ScriptFieldInitializer>& values) const 
	{
		for (const auto& [name, initializer] : values)
		{
			SetFieldValueInternal(name, initializer.m_Data.data());
		}
	}

	bool ScriptInstance::GetFieldValueInternal(const std::string& name, void* buffer) const
	{
		const ScriptField& fieldInfo = m_ScriptClass->m_SerializedFields.at(name);
		if (!fieldInfo.ClassField)
		{
			KBR_CORE_ASSERT(false, "Field {0} not found in class {1}.{2}", name, m_ScriptClass->m_ClassNamespace, m_ScriptClass->m_ClassName);
			return false;
		}

		mono_field_get_value(m_Instance, fieldInfo.ClassField, buffer);
		if (!buffer) 
		{
			KBR_CORE_ASSERT(false, "Failed to get value of field {0} in class {1}.{2}", name, m_ScriptClass->m_ClassNamespace, m_ScriptClass->m_ClassName);
			return false;
		}

		return true;
	}

	void ScriptInstance::SetFieldValueInternal(const std::string& name, const void* value) const
	{
		const ScriptField& fieldInfo = m_ScriptClass->m_SerializedFields.at(name);
		KBR_CORE_ASSERT(fieldInfo.ClassField, "Field {0} not found in class {1}.{2}", name, m_ScriptClass->m_ClassNamespace, m_ScriptClass->m_ClassName);

		mono_field_set_value(m_Instance, fieldInfo.ClassField, const_cast<void*>(value));
	}
}