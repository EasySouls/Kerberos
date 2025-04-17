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
}
