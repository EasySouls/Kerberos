#include "kbrpch.h"
#include "ScriptInstance.h"

#include "ScriptClass.h"

namespace Kerberos
{
	ScriptInstance::ScriptInstance(const Ref<ScriptClass>& scriptClass, const Entity entity)
		: m_Entity(entity), m_ScriptClass(scriptClass)
	{
		m_Instance = m_ScriptClass->Instantiate();

		m_OnCreateMethod = m_ScriptClass->GetMethod("OnCreate", 0);
		m_OnUpdateMethod = m_ScriptClass->GetMethod("OnUpdate", 1);
		m_Constructor = m_ScriptClass->GetMethod(".ctor", 1);

		UUID entityID = m_Entity.GetUUID();
		void* params[1] = { &entityID };
		m_ScriptClass->InvokeMethod(m_Constructor, m_Instance, params);
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
}