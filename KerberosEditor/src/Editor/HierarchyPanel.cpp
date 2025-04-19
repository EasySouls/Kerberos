#include "HierarchyPanel.h"

#include "Kerberos/Scene/Components.h"
#include <imgui/imgui.h>

namespace Kerberos
{
	HierarchyPanel::HierarchyPanel(const Ref<Scene>& context)
		: m_Context(context)
	{
	}

	void HierarchyPanel::SetContext(const Ref<Scene>& context) 
	{
		m_Context = context;
	}

	void HierarchyPanel::OnImGuiRender() 
	{
		ImGui::Begin("Hierarchy");
		for (const auto entityId : m_Context->m_Registry.view<entt::entity>())
		{
			Entity e{ entityId, m_Context.get() };
			DrawEntityNode(e);
		}
		ImGui::End();

		if (ImGui::IsMouseDown(0) && ImGui::IsWindowHovered())
		{
			m_SelectedEntity = {};
		}

		ImGui::Begin("Properties");
		if (m_SelectedEntity)
		{
			DrawComponents(m_SelectedEntity);
		}
		ImGui::End();
	}

	void HierarchyPanel::DrawEntityNode(const Entity& entity)
	{
		const auto& tag = entity.GetComponent<TagComponent>();

		const ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth 
			| (entity == m_SelectedEntity ? ImGuiTreeNodeFlags_Selected : 0);

		/// The entity's identifier serves as the unique ID for the ImGui tree node
		const bool opened = ImGui::TreeNodeEx(reinterpret_cast<void*>(static_cast<uint64_t>(static_cast<uint32_t>(entity))), flags, "%s", static_cast<const char*>(tag));

		if (ImGui::IsItemClicked())
		{
			m_SelectedEntity = entity;
		}

		if (opened)
		{
			ImGui::TreePop();
		}
	}

	void HierarchyPanel::DrawComponents(const Entity entity) 
	{
		if (entity.HasComponent<TagComponent>())
		{
			auto& tag = entity.GetComponent<TagComponent>().Tag;

			char buffer[256];
			strcpy_s(buffer, sizeof(buffer), tag.c_str());
			if (ImGui::InputText("Tag", buffer, sizeof(buffer)))
			{
				tag = buffer;
			}
		}
		if (entity.HasComponent<TransformComponent>())
		{
			auto& transform = entity.GetComponent<TransformComponent>();
			ImGui::Text("Transform");
			ImGui::DragFloat3("Position", &transform.Transform[3][0], 0.1f);
			ImGui::DragFloat3("Rotation", &transform.Transform[2][0], 0.1f);
			ImGui::DragFloat3("Scale", &transform.Transform[1][0], 0.1f);
		}
		if (entity.HasComponent<SpriteRendererComponent>())
		{
			auto& spriteRenderer = entity.GetComponent<SpriteRendererComponent>();
			ImGui::Text("Sprite");
			ImGui::ColorEdit4("Red", &spriteRenderer.Color.r);
			ImGui::ColorEdit4("Green", &spriteRenderer.Color.g);
			ImGui::ColorEdit4("Blue", &spriteRenderer.Color.b);
			ImGui::ColorEdit4("Alpha", &spriteRenderer.Color.a);
		}
	}
}
