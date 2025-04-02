#pragma once

#include "Kerberos/Core/Timestep.h"

#include <entt.hpp>

namespace Kerberos
{
	class Scene
	{
	public:
		Scene();
		~Scene() = default;

		void OnUpdate(Timestep ts);

		entt::entity CreateEntity();

		entt::registry& GetRegistry() { return m_Registry; }

	private:
		entt::registry m_Registry;
	};
}
