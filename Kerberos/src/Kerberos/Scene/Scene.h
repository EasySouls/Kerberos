#pragma once

#include "Kerberos/Core/Timestep.h"

#include <entt.hpp>

#include "EditorCamera.h"
#include "Kerberos/Renderer/Camera.h"

namespace JPH
{
	class PhysicsSystem;
	class TempAllocator;
	class JobSystem;
	class ObjectVsBroadPhaseLayerFilter;
	class BroadPhaseLayerInterface;
	class ObjectLayerPairFilter;
}

namespace Kerberos
{
	class Entity;
	class HierarchyPanel;

	class Scene
	{
	public:
		Scene();
		virtual ~Scene();

		void OnRuntimeStart();
		void OnRuntimeStop() const;

		void OnUpdateEditor(Timestep ts, const EditorCamera& camera, bool renderSkybox);
		void OnUpdateRuntime(Timestep ts);

		/**
		 * @brief Create an entity in the scene and assigns it a transform component
		 *
		 * @return Entity The entity created
		 */
		Entity CreateEntity(const std::string& name = std::string());

		Entity CreateEntity(const std::string& name, uint32_t id);

		/**
		 * @brief Destroy an entity in the scene
		 *
		 * @param entity The entity to destroy
		 */
		void DestroyEntity(Entity entity);

		void SetParent(Entity child, Entity parent, bool keepWorldTransform = true);
		Entity GetParent(Entity child);
		void RemoveParent(Entity child);
		const std::vector<Entity>& GetChildren(Entity parent);


		void OnViewportResize(uint32_t width, uint32_t height);

		void SetIs3D(const bool is3D) { m_Is3D = is3D; }

		Entity GetPrimaryCameraEntity();
		void CalculateEntityTransforms();

	private:
		template<typename T>
		void OnComponentAdded(Entity entity, T& component);

		void Render2DRuntime(const Camera* mainCamera, const glm::mat4& mainCameraTransform);
		void Render3DRuntime(const Camera* mainCamera, const glm::mat4& mainCameraTransform);
		void Render3DEditor(const EditorCamera& camera, bool renderSkybox);

		void UpdateChildTransforms(Entity parent, const glm::mat4& parentTransform);

	private:
		entt::registry m_Registry;

		uint32_t m_ViewportWidth = 0;
		uint32_t m_ViewportHeight = 0;

		bool m_Is3D = true;

		/// Physics related members
		/// These are pointers, since i do not want to include Jolt headers in the Scene.h file,
		/// and non-complete types are not allowed in the class definition
		
		JPH::PhysicsSystem* m_PhysicsSystem = nullptr;
		JPH::TempAllocator* m_PhysicsTempAllocator = nullptr;
		JPH::JobSystem* m_PhysicsJobSystem = nullptr;
		JPH::ObjectVsBroadPhaseLayerFilter* m_ObjectVsBroadPhaseLayerFilter = nullptr;
		JPH::BroadPhaseLayerInterface* m_BroadPhaseLayerInterface = nullptr;
		JPH::ObjectLayerPairFilter* m_ObjectVsObjectLayerFilter = nullptr;

		friend class Entity;
		friend class HierarchyPanel;
		friend class SceneSerializer;
	};
}
