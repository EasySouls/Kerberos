#include "kbrpch.h"
#include "Scene.h"
#include "Kerberos/Scene/Entity.h"
#include "Kerberos/Scene/ScriptableEntity.h"

#include "Components.h"
#include "Kerberos/Renderer/Renderer2D.h"
#include "Kerberos/Renderer/Renderer3D.h"

namespace Kerberos
{
	Scene::Scene()
	{
		m_Registry = entt::basic_registry();
	}

	Scene::~Scene() 
	{
		/// Destroy all entities in the scene
		m_Registry.clear<entt::entity>();
	}

	void Scene::OnUpdateEditor(Timestep ts, const EditorCamera& camera, const bool renderSkybox)
	{
		Render3DEditor(camera, renderSkybox);
	}

	void Scene::OnUpdateRuntime(Timestep ts)
	{
		/// Update the scripts
		{
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
		Entity entity = { m_Registry.create(), this };
		entity.AddComponent<TransformComponent>();
		entity.AddComponent<HierarchyComponent>();

		auto& tag = entity.AddComponent<TagComponent>();
		tag.Tag = name.empty() ? "Entity" : name;

		return entity;
	}

	void Scene::DestroyEntity(const Entity entity) 
	{
		m_Registry.destroy(static_cast<entt::entity>(entity));
	}

	void Scene::SetParent(const Entity child, const Entity parent, bool keepWorldTransform) 
	{
		RemoveParent(child);

		auto& childHierarchy = child.GetComponent<HierarchyComponent>();
		auto& parentHierarchy = parent.GetComponent<HierarchyComponent>();

		childHierarchy.Parent = parent;
		parentHierarchy.Children.push_back(child);
	}

	Entity Scene::GetParent(const Entity child) 
	{
		const auto& childHierarchy = child.GetComponent<HierarchyComponent>();
		if (childHierarchy.Parent)
		{
			return childHierarchy.Parent;
		}
		return {};
	}

	void Scene::RemoveParent(const Entity child) 
	{
		auto& childHierarchy = child.GetComponent<HierarchyComponent>();
		if (childHierarchy.Parent)
		{
			auto& parentHierarchy = childHierarchy.Parent.GetComponent<HierarchyComponent>();
			const auto it = std::ranges::find(parentHierarchy.Children, child);
			if (it != parentHierarchy.Children.end())
			{
				parentHierarchy.Children.erase(it);
			}
			childHierarchy.Parent = Entity{};
		}
	}

	const std::vector<Entity>& Scene::GetChildren(const Entity parent) 
	{
		auto& parentHierarchy = parent.GetComponent<HierarchyComponent>();
		return parentHierarchy.Children;
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

	void Scene::Render2DRuntime(const Camera* mainCamera, const glm::mat4& mainCameraTransform) 
	{
		Renderer2D::BeginScene(*mainCamera, mainCameraTransform);

		const auto view = m_Registry.view<TransformComponent, SpriteRendererComponent>();
		for (const auto entity : view)
		{
			auto [transform, sprite] = view.get<TransformComponent, SpriteRendererComponent>(entity);

			Renderer2D::DrawQuad(transform.GetTransform(), sprite.Color);
		}

		Renderer2D::EndScene();
	}

	void Scene::Render3DRuntime(const Camera* mainCamera, const glm::mat4& mainCameraTransform)
	{
		const DirectionalLight* sun = nullptr;
		const auto sunView = m_Registry.view<DirectionalLightComponent, TransformComponent>();
		for (const auto entity : sunView)
		{
			auto [light, transform] = sunView.get<DirectionalLightComponent, TransformComponent>(entity);
			if (light.IsEnabled)
			{
				sun = &light.Light;
				break;
			}
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

		Renderer3D::BeginScene(*mainCamera, mainCameraTransform, sun, pointLights);

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

	void Scene::Render3DEditor(const EditorCamera& camera, const bool renderSkybox) 
	{
		const DirectionalLight* sun = nullptr;
		const auto sunView = m_Registry.view<DirectionalLightComponent, TransformComponent>();
		for (const auto entity : sunView)
		{
			auto [light, transform] = sunView.get<DirectionalLightComponent, TransformComponent>(entity);
			if (light.IsEnabled)
			{
				sun = &light.Light;
				break;
			}
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

		Renderer3D::BeginScene(camera, sun, pointLights, renderSkybox);

		const auto view = m_Registry.view<TransformComponent, StaticMeshComponent>();
		for (const auto entity : view)
		{
			auto [transform, mesh] = view.get<TransformComponent, StaticMeshComponent>(entity);

			if (mesh.Visible)
			{
				Renderer3D::SubmitMesh(mesh.StaticMesh, transform.WorldTransform, mesh.MeshMaterial, mesh.MeshTexture, 1.0f, static_cast<int>(entity));
			}
		}

		Renderer3D::EndScene();
	}

	void Scene::UpdateChildTransforms(const Entity parent, const glm::mat4& parentTransform) 
	{
		auto& tsc = parent.GetComponent<TransformComponent>();
		const glm::mat4 localTransform = tsc.GetTransform();
		tsc.WorldTransform = parentTransform * localTransform;

		for (const auto child : GetChildren(parent))
		{
			UpdateChildTransforms(child, tsc.WorldTransform);
		}
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
		const auto view = m_Registry.view<TransformComponent>();
		for (const auto id : view)
		{
			const Entity entity{ id, this };
			const Entity parent = entity.GetComponent<HierarchyComponent>().Parent;

			if (!parent)
			{
				UpdateChildTransforms(entity, glm::mat4(1.0f));
			}
		}
	}

	template <typename T>
	void Scene::OnComponentAdded(Entity entity, T& component)
	{
		static_assert(false, "No template specialization found for this type");
	}

	template <>
	void Scene::OnComponentAdded<CameraComponent>(Entity entity, CameraComponent& component)
	{
		component.Camera.SetViewportSize(m_ViewportWidth, m_ViewportHeight);
	}

	template <>
	void Scene::OnComponentAdded<TransformComponent>(Entity entity, TransformComponent& component)
	{	
	}

	template <>
	void Scene::OnComponentAdded<TagComponent>(Entity entity, TagComponent& component)
	{
	}

	template <>
	void Scene::OnComponentAdded<SpriteRendererComponent>(Entity entity, SpriteRendererComponent& component)
	{
	}

	template <>
	void Scene::OnComponentAdded<NativeScriptComponent>(Entity entity, NativeScriptComponent& component)
	{
	}

	template <>
	void Scene::OnComponentAdded<StaticMeshComponent>(Entity entity, StaticMeshComponent& component)
	{}

	template <>
	void Scene::OnComponentAdded<DirectionalLightComponent>(Entity entity, DirectionalLightComponent& component)
	{
	}

	template <>
	void Scene::OnComponentAdded<PointLightComponent>(Entity entity, PointLightComponent& component)
	{
	}

	template <>
	void Scene::OnComponentAdded<SpotLightComponent>(Entity entity, SpotLightComponent& component)
	{
	}

	template <>
	void Scene::OnComponentAdded<SkyboxComponent>(Entity entity, SkyboxComponent& component)
	{}

	template <>
	void Scene::OnComponentAdded<HierarchyComponent>(Entity entity, HierarchyComponent& component)
	{}
}
