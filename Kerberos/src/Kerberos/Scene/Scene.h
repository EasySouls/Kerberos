#pragma once

#include "Components.h"
#include "EditorCamera.h"
#include "Kerberos/Renderer/Camera.h"
#include "Kerberos/Renderer/Framebuffer.h"
#include "Kerberos/Core/Timestep.h"
#include "Kerberos/Core/UUID.h"
#include "Kerberos/Physics/PhysicsSystem.h"
#include "Kerberos/Assets/Asset.h"

#include <entt.hpp>
#include <set>
#include <string_view>

namespace Kerberos
{
	class Entity;
	class HierarchyPanel;

	class Scene : public std::enable_shared_from_this<Scene>, public Asset
	{
	public:
		Scene();
		~Scene() override;

		void OnRuntimeStart();
		void OnRuntimeStop();
		void OnSimulationStart();
		void OnSimulationStop();
		void SetScenePaused(bool isPaused);

		void OnUpdateEditor(Timestep ts, const EditorCamera& camera);
		void OnUpdateSimulation(Timestep ts, const EditorCamera& camera);
		void OnUpdateRuntime(Timestep ts);

		/**
		 * @brief Create an entity in the scene and assigns it a transform component
		 *
		 * @return Entity The entity created
		 */
		Entity CreateEntity(const std::string& name = std::string());

		Entity CreateEntityWithUUID(const std::string& name, uint64_t uuid);

		/**
		 * @brief Destroy an entity in the scene
		 *
		 * @param entity The entity to destroy
		 */
		void DestroyEntity(Entity entity);

		/**
		 * @brief Duplicates an entity in the scene
		 * 
		 * It new entity gets the associated assets, so if the assets are modified, the duplicated entity will reflect those changes.
		 *
		 * @param entity The entity to duplicate
		 * @param duplicateChildren If true, duplicates the children of the entity as well
		 */
		void DuplicateEntity(Entity entity, bool duplicateChildren);

		void CreateChild(Entity entity);

		Entity GetEntityByUUID(UUID uuid) const;

		void SetParent(Entity child, Entity parent, bool keepWorldTransform = true);
		Entity GetParent(Entity child) const;
		void RemoveParent(Entity child);
		std::vector<Entity> GetChildren(Entity parent) const;

		const std::set<entt::entity>& GetRootEntities() const { return m_RootEntities; }

		void OnViewportResize(uint32_t width, uint32_t height);

		void SetIs3D(const bool is3D) { m_Is3D = is3D; }
		void SetEnableShadowMapping(const bool enable) { m_EnableShadowMapping = enable; }

		Entity GetPrimaryCameraEntity();
		void CalculateEntityTransforms();
		void CalculateEntityTransform(const Entity& entity);

		Entity FindEntityByName(std::string_view name);

		Ref<Framebuffer> GetOmniShadowMapFramebuffer() const { return m_OmniShadowMapFramebuffer; }
		Ref<Framebuffer> GetShadowMapFramebuffer() const { return m_ShadowMapFramebuffer; }
		Ref<Framebuffer> GetEditorFramebuffer() const { return m_EditorFramebuffer; }
		bool& GetOnlyRenderShadowMapIfLightHasChanged() { return m_OnlyRenderShadowMapIfLightHasChanged; }

		const PhysicsSystem& GetPhysicsSystem() const { return m_PhysicsSystem; }
		PhysicsSystem& GetPhysicsSystem() { return m_PhysicsSystem; }

		static Ref<Scene> Copy(const Ref<Scene>& other);

		AssetType GetType() override;

	private:
		template<typename T>
		void OnComponentAdded(Entity entity, T& component);

		void Render2DRuntime(const Camera* mainCamera, const glm::mat4& mainCameraTransform);
		void Render3DRuntime(const Camera* mainCamera, const glm::mat4& mainCameraTransform);
		void Render3DEditor(const EditorCamera& camera);

		void UpdateScripts(Timestep ts);

		void UpdateChildTransforms(Entity parent, const glm::mat4& parentTransform);

		bool ShouldRenderShadows(const DirectionalLightComponent* dlc) const;

		template<typename Component>
		static void CopyComponent(entt::registry& dst, entt::registry& src)
		{

		}

	private:
		entt::registry m_Registry;

		uint32_t m_ViewportWidth = 0;
		uint32_t m_ViewportHeight = 0;

		bool m_IsScenePaused = false;

		bool m_Is3D = true;
		bool m_EnableShadowMapping = true;
		bool m_OnlyRenderShadowMapIfLightHasChanged = false;

		Ref<Framebuffer> m_OmniShadowMapFramebuffer;
		Ref<Framebuffer> m_ShadowMapFramebuffer;
		Ref<Framebuffer> m_EditorFramebuffer;

		std::unordered_map<UUID, Entity> m_UUIDToEntityMap;

		std::set<entt::entity> m_RootEntities;

		PhysicsSystem m_PhysicsSystem;

		friend class Entity;
		friend class PhysicsSystem;
		friend class HierarchyPanel;
		friend class SceneSerializer;
	};
}
