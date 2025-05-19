#pragma once

#include "Kerberos/Core/Timestep.h"

#include <entt.hpp>


namespace Kerberos
{
	class Entity;
	class HierarchyPanel;

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

		/**
		 * @brief Destroy an entity in the scene
		 *
		 * @param entity The entity to destroy
		 */
		void DestroyEntity(Entity entity);


		void OnViewportResize(uint32_t width, uint32_t height);
		
	private:
		template<typename T>
		void OnComponentAdded(Entity entity, T& component);

	private:
		entt::registry m_Registry;

		uint32_t m_ViewportWidth = 0;
		uint32_t m_ViewportHeight = 0;

		friend class Entity;
		friend class HierarchyPanel;
	};
}
