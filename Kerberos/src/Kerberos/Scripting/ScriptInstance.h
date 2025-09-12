#pragma once

#include "Kerberos/Core.h"
#include "Kerberos/Scene/Entity.h"
#include "ScriptClass.h"

extern "C" {
	typedef struct _MonoClass		MonoClass;
	typedef struct _MonoObject		MonoObject;
	typedef struct _MonoMethod		MonoMethod;
}

namespace Kerberos 
{
	class ScriptInstance
	{
	public:
		explicit ScriptInstance(const Ref<ScriptClass>& scriptClass, Entity entity);

		void InvokeOnCreate() const;
		void InvokeOnUpdate(float deltaTime) const;

	private:
		Entity m_Entity;
		Ref<ScriptClass> m_ScriptClass = nullptr;

		MonoObject* m_Instance = nullptr;
		MonoMethod* m_OnCreateMethod = nullptr;
		MonoMethod* m_OnUpdateMethod = nullptr;
		/// Constructor with the UUID parameter
		MonoMethod* m_Constructor = nullptr;
	};
}