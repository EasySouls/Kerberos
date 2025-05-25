#pragma once

#include "Kerberos/Core/Timestep.h"

#include <entt.hpp>

#include "EditorCamera.h"
#include "Kerberos/Renderer/Camera.h"


namespace Kerberos
{
	class Entity;
	class HierarchyPanel;

	class Scene
	{
	public:
		Scene();
		virtual ~Scene();

		void OnUpdateEditor(Timestep ts, const EditorCamera& camera, bool renderSkybox);
		void OnUpdateRuntime(Timestep ts);

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

		void SetIs3D(const bool is3D) { m_Is3D = is3D; }

		Entity GetPrimaryCameraEntity();

	private:
		template<typename T>
		void OnComponentAdded(Entity entity, T& component);

		void Render2DRuntime(const Camera* mainCamera, const glm::mat4& mainCameraTransform);

		void Render3DRuntime(const Camera* mainCamera, const glm::mat4& mainCameraTransform);

		void Render3DEditor(const EditorCamera& camera, bool renderSkybox);

	private:
		entt::registry m_Registry;

		uint32_t m_ViewportWidth = 0;
		uint32_t m_ViewportHeight = 0;

		bool m_Is3D = true;

		friend class Entity;
		friend class HierarchyPanel;
		friend class SceneSerializer;
	};
}
