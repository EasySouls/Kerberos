#pragma once

#include "Kerberos/Core/Timestep.h"

#include <entt.hpp>

#include "Kerberos/Renderer/Camera.h"


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

		void Toggle3DMode() { m_Is3D = !m_Is3D; }
		
	private:
		template<typename T>
		void OnComponentAdded(Entity entity, T& component);

		void Render2D(const Camera* mainCamera, const glm::mat4& mainCameraTransform);

		void Render3D(const Camera* mainCamera, const glm::mat4& mainCameraTransform);

	private:
		entt::registry m_Registry;

		uint32_t m_ViewportWidth = 0;
		uint32_t m_ViewportHeight = 0;

		bool m_Is3D = false;

		friend class Entity;
		friend class HierarchyPanel;
	};
}
