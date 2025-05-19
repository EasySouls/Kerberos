#pragma once
#include "Entity.h"

namespace Kerberos
{
	class ScriptableEntity
	{
	public:
		virtual ~ScriptableEntity() = default;

		template<typename T>
		T& GetComponent()
		{
			return m_Entity.GetComponent<T>();
		}

		virtual void OnCreate() {}

		virtual void OnDestroy() {}

		virtual void OnUpdate(const Timestep ts) {}

	private:
		Entity m_Entity;

		friend class Scene;
	};
}
