#pragma once

#include "Kerberos/Core/Timestep.h"
#include "Components.h"

#include <entt.hpp>


namespace Kerberos
{
	class Entity;

	class Scene
	{
	public:
		Scene();
		~Scene() = default;

		void OnUpdate(Timestep ts);

		/**
		 * @brief Create an entity in the scene and assigns it a transform component
		 *
		 * @return Entity The entity created
		 */
		Entity CreateEntity(const std::string& name = std::string());

	private:
		entt::registry m_Registry;

		friend class Entity;
	};
}
