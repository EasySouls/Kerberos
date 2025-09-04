#include "kbrpch.h"

import Components.PhysicsComponents;

#include "Scene.h"
#include "Kerberos/Scene/Entity.h"
#include "Kerberos/Scene/ScriptableEntity.h"

#include "Components.h"
#include "Kerberos/Renderer/Renderer2D.h"
#include "Kerberos/Renderer/Renderer3D.h"
#include "Kerberos/Physics/Utils.h"

#include <glm/gtx/matrix_decompose.hpp>

#include "Kerberos/Assets/AssetManager.h"
#include "Kerberos/Renderer/RenderCommand.h"
#include "Kerberos/Scripting/ScriptEngine.h"

#define USE_MAP_FOR_UUID 1

namespace Kerberos
{
	Scene::Scene()
	{
		m_Registry = entt::basic_registry();

		m_ShadowMapFramebuffer = Framebuffer::Create(FramebufferSpecification{
			.Width = 1024,
			.Height = 1024,
			.Attachments = {
				{ FramebufferTextureFormat::DEPTH24 }
			}
			});
		m_ShadowMapFramebuffer->SetDebugName("ShadowMapFramebuffer");

		m_EditorFramebuffer = Framebuffer::Create(FramebufferSpecification{
			.Width = 1280,
			.Height = 720,
			.Attachments = {
				{ FramebufferTextureFormat::RGBA8 },
				{ FramebufferTextureFormat::RED_INTEGER },
				{ FramebufferTextureFormat::DEPTH24STENCIL8 }
			}
			});
		m_EditorFramebuffer->SetDebugName("EditorFramebuffer");
	}

	Scene::~Scene()
	{
		/// Destroy all entities in the scene
		m_Registry.clear<entt::entity>();
	}

	void Scene::OnRuntimeStart()
	{
		KBR_PROFILE_FUNCTION();

		m_PhysicsSystem.Initialize(shared_from_this());

		ScriptEngine::OnRuntimeStart(shared_from_this());

		/// Instantiate all scripts

		m_Registry.view<ScriptComponent>().each([this](auto enttId, ScriptComponent& script) {
			Entity entity{ enttId, this };
			ScriptEngine::OnCreateEntity(entity);
		});
	}

	void Scene::OnRuntimeStop()
	{
		m_PhysicsSystem.Cleanup();

		ScriptEngine::OnRuntimeStop();
	}

	void Scene::OnUpdateEditor(Timestep ts, const EditorCamera& camera)
	{
		Render3DEditor(camera);
	}

	void Scene::OnUpdateRuntime(Timestep ts)
	{
		KBR_PROFILE_FUNCTION();

		/// Update the scripts
		{
			KBR_PROFILE_SCOPE("Scene::OnUpdateRuntime - Scripts update");

			m_Registry.view<NativeScriptComponent>().each([this, ts](auto entity, const NativeScriptComponent& script)
				{
					if (!script.Instance)
					{
						script.Instantiate();
						script.Instance->m_Entity = Entity{ entity, this };
						script.Instance->OnCreate();
					}

					script.Instance->OnUpdate(ts);
				});
		}

		m_PhysicsSystem.Update(ts);

		/// Render the scene

		const Camera* mainCamera = nullptr;
		glm::mat4 mainCameraTransform;

		{
			const auto view = m_Registry.view<CameraComponent, TransformComponent>();
			for (const auto entity : view)
			{
				auto [camera, transform] = view.get<CameraComponent, TransformComponent>(entity);
				if (camera.IsPrimary)
				{
					mainCamera = &camera.Camera;
					mainCameraTransform = transform.GetTransform();
					break;
				}
			}
		}

		if (mainCamera)
		{
			if (m_Is3D)
			{
				Render3DRuntime(mainCamera, mainCameraTransform);
			}
			else
			{
				Render2DRuntime(mainCamera, mainCameraTransform);
			}
		}
	}

	Entity Scene::CreateEntity(const std::string& name)
	{
		const auto enttId = m_Registry.create();
		Entity entity = { enttId, this };
		m_RootEntities.insert(enttId);

		entity.AddComponent<TransformComponent>();
		entity.AddComponent<HierarchyComponent>();

		auto& tag = entity.AddComponent<TagComponent>();
		tag.Tag = name.empty() ? "Entity" : name;

		const auto& idComp = entity.AddComponent<IDComponent>();
#if USE_MAP_FOR_UUID
		m_UUIDToEntityMap[idComp.ID] = entity;
#endif

		return entity;
	}

	Entity Scene::CreateEntityWithUUID(const std::string& name, const uint64_t uuid)
	{
		const auto enttId = m_Registry.create();
		Entity entity = { enttId, this };
		m_RootEntities.insert(enttId);

		entity.AddComponent<TransformComponent>();
		entity.AddComponent<HierarchyComponent>();

		auto& idComp = entity.AddComponent<IDComponent>();
		idComp.ID = UUID(uuid);

		auto& tag = entity.AddComponent<TagComponent>();
		tag.Tag = name.empty() ? "Entity" : name;

#if USE_MAP_FOR_UUID
		m_UUIDToEntityMap[idComp.ID] = entity;
#endif

		return entity;
	}

	void Scene::DestroyEntity(const Entity entity)
	{
		const entt::entity enttId = static_cast<entt::entity>(entity);
		if (m_RootEntities.contains(enttId))
		{
			m_RootEntities.erase(enttId);
		}

		/// Destroy all children entities
		const auto children = GetChildren(entity);
		for (const auto& child : children)
		{
			DestroyEntity(child);
		}

		m_Registry.destroy(enttId);
	}

	void Scene::DuplicateEntity(const Entity entity, const bool duplicateChildren)
	{
		KBR_PROFILE_FUNCTION();

		const std::string name = entity.GetComponent<TagComponent>().Tag;
		const std::string newName = name + " Copy";

		Entity newEntity = CreateEntity(newName);

		newEntity.GetComponent<TransformComponent>() = entity.GetComponent<TransformComponent>();

		if (entity.HasComponent<SpriteRendererComponent>())
		{
			newEntity.AddComponent<SpriteRendererComponent>(entity.GetComponent<SpriteRendererComponent>());
		}
		if (entity.HasComponent<CameraComponent>())
		{
			newEntity.AddComponent<CameraComponent>(entity.GetComponent<CameraComponent>());
			auto& camera = newEntity.GetComponent<CameraComponent>();
			camera.Camera.SetViewportSize(m_ViewportWidth, m_ViewportHeight);
		}
		if (entity.HasComponent<NativeScriptComponent>())
		{
			auto& script = entity.GetComponent<NativeScriptComponent>();
			/// Instantiate the script instance, so that script.Instance is not nullptr,
			/// when calling script.Instantiate() in the new entity.
			script.Instantiate();

			auto& newScriptComp = newEntity.AddComponent<NativeScriptComponent>(script);
			newScriptComp.Instantiate = [&]() { newScriptComp.Instance = script.Instance; };
			newScriptComp.Destroy = [&]()
				{
					delete newScriptComp.Instance;
					newScriptComp.Instance = nullptr;
				};
		}
		if (entity.HasComponent<StaticMeshComponent>())
		{
			newEntity.AddComponent<StaticMeshComponent>(entity.GetComponent<StaticMeshComponent>());
		}
		if (entity.HasComponent<RigidBody3DComponent>())
		{
			newEntity.AddComponent<RigidBody3DComponent>(entity.GetComponent<RigidBody3DComponent>());
			auto& rigidBody = newEntity.GetComponent<RigidBody3DComponent>();
			rigidBody.RuntimeBody = nullptr;
		}
		if (entity.HasComponent<BoxCollider3DComponent>())
		{
			newEntity.AddComponent<BoxCollider3DComponent>(entity.GetComponent<BoxCollider3DComponent>());
		}
		if (entity.HasComponent<SphereCollider3DComponent>())
		{
			newEntity.AddComponent<SphereCollider3DComponent>(entity.GetComponent<SphereCollider3DComponent>());
		}
		if (entity.HasComponent<CapsuleCollider3DComponent>())
		{
			newEntity.AddComponent<CapsuleCollider3DComponent>(entity.GetComponent<CapsuleCollider3DComponent>());
		}
		if (entity.HasComponent<MeshCollider3DComponent>())
		{
			newEntity.AddComponent<MeshCollider3DComponent>(entity.GetComponent<MeshCollider3DComponent>());
		}
		if (entity.HasComponent<DirectionalLightComponent>())
		{
			newEntity.AddComponent<DirectionalLightComponent>(entity.GetComponent<DirectionalLightComponent>());
		}
		if (entity.HasComponent<PointLightComponent>())
		{
			newEntity.AddComponent<PointLightComponent>(entity.GetComponent<PointLightComponent>());
		}
		if (entity.HasComponent<SpotLightComponent>())
		{
			newEntity.AddComponent<SpotLightComponent>(entity.GetComponent<SpotLightComponent>());
		}

		if (duplicateChildren)
		{
			const auto children = GetChildren(entity);
			for (const auto& child : children)
			{
				DuplicateEntity(child, true);
				SetParent(child, newEntity, false);
			}
		}
	}

	void Scene::CreateChild(const Entity entity)
	{
		const Entity child = CreateEntity("Unnamed");
		SetParent(child, entity);
	}

	Entity Scene::GetEntityByUUID(const UUID uuid) const
	{
		KBR_PROFILE_FUNCTION();

#if USE_MAP_FOR_UUID

		return m_UUIDToEntityMap.at(uuid);
#else
		const auto view = m_Registry.view<IDComponent>();
		for (auto entity : view)
		{
			auto id = view.get<IDComponent>(entity).ID;
			if (id == uuid)
			{
				return { entity, this };
			}
		}

		return {};
#endif
	}

	void Scene::SetParent(const Entity child, const Entity parent, bool keepWorldTransform)
	{
		RemoveParent(child);

		m_RootEntities.erase(static_cast<entt::entity>(child));

		auto& childHierarchy = child.GetComponent<HierarchyComponent>();
		auto& parentHierarchy = parent.GetComponent<HierarchyComponent>();

		childHierarchy.Parent = parent.GetUUID();
		parentHierarchy.Children.emplace_back(child.GetUUID());
	}

	Entity Scene::GetParent(const Entity child) const
	{
		KBR_PROFILE_FUNCTION();

		const auto& childHierarchy = child.GetComponent<HierarchyComponent>();
		if (childHierarchy.Parent.IsValid())
		{
			return GetEntityByUUID(childHierarchy.Parent);
		}
		return {};
	}

	void Scene::RemoveParent(const Entity child)
	{
		KBR_PROFILE_FUNCTION();

		auto& childHierarchy = child.GetComponent<HierarchyComponent>();
		if (childHierarchy.Parent.IsValid())
		{
			auto& parentHierarchy = GetEntityByUUID(childHierarchy.Parent).GetComponent<HierarchyComponent>();
			const auto it = std::ranges::find(parentHierarchy.Children, child.GetUUID());
			if (it != parentHierarchy.Children.end())
			{
				parentHierarchy.Children.erase(it);
			}

			childHierarchy.Parent = UUID::Invalid();
			/// The child is a root entity now
			m_RootEntities.insert(static_cast<entt::entity>(child));
		}
	}

	std::vector<Entity> Scene::GetChildren(const Entity parent) const
	{
		KBR_PROFILE_FUNCTION();

		const auto& parentHierarchy = parent.GetComponent<HierarchyComponent>();

		std::vector<Entity> children;
		for (const auto& child : parentHierarchy.Children)
		{
			Entity entity = GetEntityByUUID(child);
			children.push_back(entity);
		}
		return children;
	}

	void Scene::OnViewportResize(const uint32_t width, const uint32_t height)
	{
		m_ViewportHeight = height;
		m_ViewportWidth = width;

		/// Resize the non-fixed aspect ratio cameras
		const auto view = m_Registry.view<CameraComponent>();
		for (const auto entity : view)
		{
			auto& cameraComponent = view.get<CameraComponent>(entity);
			if (!cameraComponent.FixedAspectRatio)
			{
				cameraComponent.Camera.SetViewportSize(width, height);
			}
		}
	}

	Ref<Scene> Scene::Copy(const Ref<Scene>& other)
	{
		Ref<Scene> newScene = CreateRef<Scene>();

		newScene->m_ViewportWidth = other->m_ViewportWidth;
		newScene->m_ViewportHeight = other->m_ViewportHeight;

		auto& sourceRegistry = other->m_Registry;
		auto& newRegistry = newScene->m_Registry;

		const auto idView = sourceRegistry.view<IDComponent>();
		for (const auto& entity : idView)
		{
			UUID sourceID = sourceRegistry.get<IDComponent>(entity).ID;
			const auto& tag = sourceRegistry.get<TagComponent>(entity).Tag;
			Entity newEntity = newScene->CreateEntityWithUUID(tag, sourceID);

			/// Copy all components

			newEntity.GetComponent<TransformComponent>() = sourceRegistry.get<TransformComponent>(entity);

			if (sourceRegistry.all_of<SpriteRendererComponent>(entity))
			{
				newEntity.AddComponent<SpriteRendererComponent>(sourceRegistry.get<SpriteRendererComponent>(entity));
			}
			if (sourceRegistry.all_of<CameraComponent>(entity))
			{
				auto& cameraComp = sourceRegistry.get<CameraComponent>(entity);
				newEntity.AddComponent<CameraComponent>(cameraComp);
				newEntity.GetComponent<CameraComponent>().Camera.SetViewportSize(newScene->m_ViewportWidth, newScene->m_ViewportHeight);
			}
			if (sourceRegistry.all_of<ScriptComponent>(entity))
			{
				newEntity.AddComponent<ScriptComponent>(sourceRegistry.get<ScriptComponent>(entity));
			}
			if (sourceRegistry.all_of<NativeScriptComponent>(entity))
			{
				auto& scriptComp = sourceRegistry.get<NativeScriptComponent>(entity);
				/// Instantiate the script instance, so that script.Instance is not nullptr,
				/// when calling script.Instantiate() in the new entity.
				scriptComp.Instantiate();

				auto& newScriptComp = newEntity.AddComponent<NativeScriptComponent>();
				newScriptComp.Instantiate = [&]() { newScriptComp.Instance = scriptComp.Instance; };
				newScriptComp.Destroy = [&]()
					{
						delete newScriptComp.Instance;
						newScriptComp.Instance = nullptr;
					};
			}
			if (sourceRegistry.all_of<StaticMeshComponent>(entity))
			{
				newEntity.AddComponent<StaticMeshComponent>(sourceRegistry.get<StaticMeshComponent>(entity));
			}
			if (sourceRegistry.all_of<RigidBody3DComponent>(entity))
			{
				auto& rigidBodyComp = sourceRegistry.get<RigidBody3DComponent>(entity);
				newEntity.AddComponent<RigidBody3DComponent>(rigidBodyComp);
				newEntity.GetComponent<RigidBody3DComponent>().RuntimeBody = nullptr; // Reset runtime body
			}
			if (sourceRegistry.all_of<BoxCollider3DComponent>(entity))
			{
				newEntity.AddComponent<BoxCollider3DComponent>(sourceRegistry.get<BoxCollider3DComponent>(entity));
			}
			if (sourceRegistry.all_of<SphereCollider3DComponent>(entity))
			{
				newEntity.AddComponent<SphereCollider3DComponent>(sourceRegistry.get<SphereCollider3DComponent>(entity));
			}
			if (sourceRegistry.all_of<CapsuleCollider3DComponent>(entity))
			{
				newEntity.AddComponent<CapsuleCollider3DComponent>(sourceRegistry.get<CapsuleCollider3DComponent>(entity));
			}
			if (sourceRegistry.all_of<MeshCollider3DComponent>(entity))
			{
				newEntity.AddComponent<MeshCollider3DComponent>(sourceRegistry.get<MeshCollider3DComponent>(entity));
			}
			if (sourceRegistry.all_of<DirectionalLightComponent>(entity))
			{
				newEntity.AddComponent<DirectionalLightComponent>(sourceRegistry.get<DirectionalLightComponent>(entity));
			}
			if (sourceRegistry.all_of<PointLightComponent>(entity))
			{
				newEntity.AddComponent<PointLightComponent>(sourceRegistry.get<PointLightComponent>(entity));
			}
			if (sourceRegistry.all_of<SpotLightComponent>(entity))
			{
				newEntity.AddComponent<SpotLightComponent>(sourceRegistry.get<SpotLightComponent>(entity));
			}
			if (sourceRegistry.all_of<EnvironmentComponent>(entity))
			{
				newEntity.AddComponent<EnvironmentComponent>(sourceRegistry.get<EnvironmentComponent>(entity));
			}
		}

		return newScene;
	}


	void Scene::Render2DRuntime(const Camera* mainCamera, const glm::mat4& mainCameraTransform)
	{
		Renderer2D::BeginScene(*mainCamera, mainCameraTransform);

		const auto view = m_Registry.view<TransformComponent, SpriteRendererComponent>();
		for (const auto entity : view)
		{
			auto [transform, sprite] = view.get<TransformComponent, SpriteRendererComponent>(entity);

			Renderer2D::DrawQuad(transform.WorldTransform, sprite.Color);
		}

		Renderer2D::EndScene();
	}

	void Scene::Render3DRuntime(const Camera* mainCamera, const glm::mat4& mainCameraTransform)
	{
		KBR_PROFILE_FUNCTION();

		DirectionalLightComponent* dlc = nullptr;
		const auto sunView = m_Registry.view<DirectionalLightComponent, TransformComponent>();
		for (const auto entity : sunView)
		{
			auto [light, transform] = sunView.get<DirectionalLightComponent, TransformComponent>(entity);
			if (light.IsEnabled)
			{
				dlc = &light;
				break;
			}
		}

		if (ShouldRenderShadows(dlc))
		{
			ShadowMapSettings shadowSettings;
			shadowSettings.Resolution = 1024;
			shadowSettings.OrthoSize = 15.0f;
			shadowSettings.NearPlane = 1.0f;
			shadowSettings.FarPlane = 100.0f;
			shadowSettings.EnableShadows = true;

			Renderer3D::BeginShadowPass(dlc->Light, shadowSettings, m_ShadowMapFramebuffer);

			/// Render all shadow-casting meshes
			const auto meshView = m_Registry.view<StaticMeshComponent, TransformComponent>();
			for (auto entity : meshView)
			{
				auto& meshComp = meshView.get<StaticMeshComponent>(entity);
				auto& transformComp = meshView.get<TransformComponent>(entity);

				if (meshComp.StaticMesh && meshComp.MeshMaterial && meshComp.Visible)
				{
					Renderer3D::SubmitMesh(meshComp.StaticMesh, transformComp.WorldTransform,
						meshComp.MeshMaterial, meshComp.MeshTexture, 1.0f,
						static_cast<int>(entity), meshComp.CastShadows);
				}
			}

			dlc->NeedsUpdate = false;

			Renderer3D::EndPass();
		}

		std::vector<PointLight> pointLights;
		const auto pointLightView = m_Registry.view<PointLightComponent, TransformComponent>();
		for (const auto entity : pointLightView)
		{
			auto [light, transform] = pointLightView.get<PointLightComponent, TransformComponent>(entity);
			if (light.IsEnabled)
			{
				pointLights.push_back(light.Light);
			}
		}

		Ref<TextureCube> skyboxTexture = nullptr;
		const auto skyboxView = m_Registry.view<EnvironmentComponent>();
		for (const auto entity : skyboxView)
		{
			const auto& skybox = skyboxView.get<EnvironmentComponent>(entity);
			if (skybox.IsSkyboxEnabled && skybox.SkyboxTexture)
			{
				skyboxTexture = AssetManager::GetAsset<TextureCube>(skybox.SkyboxTexture);
				break;
			}
		}

		m_EditorFramebuffer->Bind();

		RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1 });
		RenderCommand::Clear();

		/// Clear our entity ID attachment to -1, so when rendering entities they fill that space with their entity ID,
		/// and empty spacces will have -1, signaling that there is no entity.
		/// Used for mouse picking.
		m_EditorFramebuffer->ClearAttachment(1, -1);

		Renderer3D::BeginGeometryPass(*mainCamera, mainCameraTransform, &dlc->Light, pointLights, skyboxTexture);

		const auto view = m_Registry.view<TransformComponent, StaticMeshComponent>();
		for (const auto entity : view)
		{
			auto [transform, mesh] = view.get<TransformComponent, StaticMeshComponent>(entity);

			if (mesh.Visible)
			{
				Renderer3D::SubmitMesh(mesh.StaticMesh, transform.WorldTransform, mesh.MeshMaterial, mesh.MeshTexture);
			}
		}

		Renderer3D::EndScene();
	}

	void Scene::Render3DEditor(const EditorCamera& camera)
	{
		DirectionalLightComponent* dlc = nullptr;
		const auto sunView = m_Registry.view<DirectionalLightComponent, TransformComponent>();
		for (const auto entity : sunView)
		{
			auto [light, transform] = sunView.get<DirectionalLightComponent, TransformComponent>(entity);
			if (light.IsEnabled)
			{
				dlc = &light;
				break;
			}
		}

		if (ShouldRenderShadows(dlc))
		{
			ShadowMapSettings shadowSettings;
			shadowSettings.Resolution = 1024;
			shadowSettings.OrthoSize = 15.0f;
			shadowSettings.NearPlane = 1.0f;
			shadowSettings.FarPlane = 100.0f;
			shadowSettings.EnableShadows = true;

			Renderer3D::BeginShadowPass(dlc->Light, shadowSettings, m_ShadowMapFramebuffer);

			/// Render all shadow-casting meshes
			const auto meshView = m_Registry.view<StaticMeshComponent, TransformComponent>();
			for (auto entity : meshView)
			{
				auto& meshComp = meshView.get<StaticMeshComponent>(entity);
				auto& transformComp = meshView.get<TransformComponent>(entity);

				if (meshComp.StaticMesh && meshComp.MeshMaterial && meshComp.Visible)
				{
					Renderer3D::SubmitMesh(meshComp.StaticMesh, transformComp.WorldTransform,
						meshComp.MeshMaterial, meshComp.MeshTexture, 1.0f,
						static_cast<int>(entity), meshComp.CastShadows);
				}
			}

			dlc->NeedsUpdate = false;

			Renderer3D::EndPass();
		}

		std::vector<PointLight> pointLights;
		const auto pointLightView = m_Registry.view<PointLightComponent, TransformComponent>();
		for (const auto entity : pointLightView)
		{
			auto [light, transform] = pointLightView.get<PointLightComponent, TransformComponent>(entity);
			if (light.IsEnabled)
			{
				pointLights.push_back(light.Light);
			}
		}

		Ref<TextureCube> skyboxTexture = nullptr;
		const auto skyboxView = m_Registry.view<EnvironmentComponent>();
		for (const auto entity : skyboxView)
		{
			const auto& skybox = skyboxView.get<EnvironmentComponent>(entity);
			if (skybox.IsSkyboxEnabled && skybox.SkyboxTexture)
			{
				skyboxTexture = AssetManager::GetAsset<TextureCube>(skybox.SkyboxTexture);
				break;
			}
		}

		m_EditorFramebuffer->Bind();

		RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1 });
		RenderCommand::Clear();

		/// Clear our entity ID attachment to -1, so when rendering entities they fill that space with their entity ID,
		/// and empty spacces will have -1, signaling that there is no entity.
		/// Used for mouse picking.
		m_EditorFramebuffer->ClearAttachment(1, -1);

		Renderer3D::BeginGeometryPass(camera, &dlc->Light, pointLights, skyboxTexture);

		const auto view = m_Registry.view<TransformComponent, StaticMeshComponent>();
		for (const auto entity : view)
		{
			auto [transform, mesh] = view.get<TransformComponent, StaticMeshComponent>(entity);

			if (mesh.Visible)
			{
				Renderer3D::SubmitMesh(mesh.StaticMesh, transform.WorldTransform, mesh.MeshMaterial, mesh.MeshTexture, 1.0f, static_cast<int>(entity), mesh.CastShadows);
			}
		}

		Renderer3D::EndPass();
		Renderer3D::EndScene();
	}

	void Scene::UpdateChildTransforms(const Entity parent, const glm::mat4& parentTransform)
	{
		auto& tsc = parent.GetComponent<TransformComponent>();
		const glm::mat4 localTransform = tsc.GetTransform();
		tsc.WorldTransform = parentTransform * localTransform;

		for (const auto& child : GetChildren(parent))
		{
			UpdateChildTransforms(child, tsc.WorldTransform);
		}
	}

	bool Scene::ShouldRenderShadows(const DirectionalLightComponent* dlc) const
	{
		if (m_OnlyRenderShadowMapIfLightHasChanged)
		{
			return m_EnableShadowMapping && dlc && dlc->IsEnabled && dlc->CastShadows && dlc->NeedsUpdate;
		}
		return m_EnableShadowMapping && dlc && dlc->IsEnabled && dlc->CastShadows;
	}

	Entity Scene::GetPrimaryCameraEntity()
	{
		const auto view = m_Registry.view<CameraComponent>();
		for (const auto entity : view)
		{
			const auto& camera = view.get<CameraComponent>(entity);
			if (camera.IsPrimary)
			{
				return Entity{ entity, this };
			}
		}

		return {};
	}

	void Scene::CalculateEntityTransforms()
	{
		KBR_PROFILE_FUNCTION();

		const auto view = m_Registry.view<TransformComponent>();
		for (const auto id : view)
		{
			const Entity entity{ id, this };
			const UUID parentUUID = entity.GetComponent<HierarchyComponent>().Parent;

			/// Only calculate the transforms of root entities, since the UpdateChildTransforms
			/// function will handle all the children
			if (!parentUUID.IsValid())
			{
				UpdateChildTransforms(entity, glm::mat4(1.0f));
			}
		}
	}

	void Scene::CalculateEntityTransform(const Entity& entity)
	{
		UpdateChildTransforms(entity, glm::mat4(1.0f));
	}

	template <typename T>
	void Scene::OnComponentAdded(Entity entity, T& component)
	{
		static_assert(sizeof(T) == 0, "No template specialization found for this type");
	}

	template <>
	void Scene::OnComponentAdded<CameraComponent>(Entity entity, CameraComponent& component)
	{
		component.Camera.SetViewportSize(m_ViewportWidth, m_ViewportHeight);
	}

	template <>
	void Scene::OnComponentAdded<IDComponent>(Entity entity, IDComponent& component)
	{}

	template <>
	void Scene::OnComponentAdded<TransformComponent>(Entity entity, TransformComponent& component)
	{}

	template <>
	void Scene::OnComponentAdded<TagComponent>(Entity entity, TagComponent& component)
	{}

	template <>
	void Scene::OnComponentAdded<SpriteRendererComponent>(Entity entity, SpriteRendererComponent& component)
	{}

	template <>
	void Scene::OnComponentAdded<ScriptComponent>(Entity entity, ScriptComponent& component)
	{}

	template <>
	void Scene::OnComponentAdded<NativeScriptComponent>(Entity entity, NativeScriptComponent& component)
	{}

	template <>
	void Scene::OnComponentAdded<StaticMeshComponent>(Entity entity, StaticMeshComponent& component)
	{}

	template <>
	void Scene::OnComponentAdded<DirectionalLightComponent>(Entity entity, DirectionalLightComponent& component)
	{}

	template <>
	void Scene::OnComponentAdded<PointLightComponent>(Entity entity, PointLightComponent& component)
	{}

	template <>
	void Scene::OnComponentAdded<SpotLightComponent>(Entity entity, SpotLightComponent& component)
	{}

	template <>
	void Scene::OnComponentAdded<HierarchyComponent>(Entity entity, HierarchyComponent& component)
	{}

	template <>
	void Scene::OnComponentAdded<RigidBody3DComponent>(Entity entity, RigidBody3DComponent& component)
	{}

	template <>
	void Scene::OnComponentAdded<BoxCollider3DComponent>(Entity entity, BoxCollider3DComponent& component)
	{}

	template <>
	void Scene::OnComponentAdded<SphereCollider3DComponent>(Entity entity, SphereCollider3DComponent& component)
	{}

	template <>
	void Scene::OnComponentAdded<CapsuleCollider3DComponent>(Entity entity, CapsuleCollider3DComponent& component)
	{}

	template <>
	void Scene::OnComponentAdded<MeshCollider3DComponent>(const Entity entity, MeshCollider3DComponent& component)
	{
		const auto& smc = entity.GetComponent<StaticMeshComponent>();
		if (smc.StaticMesh == nullptr)
		{
			KBR_CORE_ERROR("MeshCollider3DComponent on entity {} does not have a valid static mesh!", entity.GetComponent<TagComponent>().Tag);
			return;
		}

		component.Mesh = smc.StaticMesh;
	}

	template <>
	void Scene::OnComponentAdded<EnvironmentComponent>(Entity entity, EnvironmentComponent& component)
	{}
}
