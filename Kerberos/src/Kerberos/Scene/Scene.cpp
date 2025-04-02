#include "kbrpch.h"
#include "Scene.h"
#include "Entity.h"

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
		const auto group = m_Registry.group<TransformComponent>(entt::get<SpriteRendererComponent>);
		for (const auto entity : group)
		{
			auto [transform, sprite] = group.get<TransformComponent, SpriteRendererComponent>(entity);

			Renderer2D::DrawQuad(transform.Transform, sprite.Color);
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

}
