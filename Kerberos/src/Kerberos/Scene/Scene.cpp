#include "kbrpch.h"
#include "Scene.h"
#include "Kerberos/Scene/Entity.h"
#include "Kerberos/Scene/ScriptableEntity.h"

#include "Components.h"
#include "Kerberos/Renderer/Renderer2D.h"

namespace Kerberos
{
	Scene::Scene()
	{
		m_Registry = entt::basic_registry();
	}

	void Scene::OnUpdate(Timestep ts)
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
			const auto group = m_Registry.group<CameraComponent>(entt::get<TransformComponent>);
			for (const auto entity : group)
			{
				auto [camera, transform] = group.get<CameraComponent, TransformComponent>(entity);
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
			Renderer2D::BeginScene(*mainCamera, mainCameraTransform);

			const auto group = m_Registry.group<TransformComponent>(entt::get<SpriteRendererComponent>);
			for (const auto entity : group)
			{
				auto [transform, sprite] = group.get<TransformComponent, SpriteRendererComponent>(entity);

				Renderer2D::DrawQuad(transform.GetTransform(), sprite.Color);
			}

			Renderer2D::EndScene();
		}
	}

	Entity Scene::CreateEntity(const std::string& name)
	{
		Entity entity = { m_Registry.create(), this };
		entity.AddComponent<TransformComponent>();

		auto& tag = entity.AddComponent<TagComponent>();
		tag.Tag = name.empty() ? "Entity" : name;

		return entity;
	}

	void Scene::DestroyEntity(const Entity entity) 
	{
		m_Registry.destroy(static_cast<entt::entity>(entity));
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
}
