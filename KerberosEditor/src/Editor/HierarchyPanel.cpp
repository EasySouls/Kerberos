#include "HierarchyPanel.h"

#include <glm/gtc/type_ptr.hpp>

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

		/// If an empty space is clicked, deselect the entity
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
			if (ImGui::TreeNodeEx(reinterpret_cast<void*>(typeid(TagComponent).hash_code()), ImGuiTreeNodeFlags_DefaultOpen, "Tag"))
			{
				auto& tag = entity.GetComponent<TagComponent>().Tag;

				char buffer[256];
				strcpy_s(buffer, sizeof(buffer), tag.c_str());
				if (ImGui::InputText("Tag", buffer, sizeof(buffer)))
				{
					tag = buffer;
				}

				ImGui::TreePop();
			}
			
		}
		if (entity.HasComponent<TransformComponent>())
		{
			if (ImGui::TreeNodeEx(reinterpret_cast<void*>(typeid(TransformComponent).hash_code()), ImGuiTreeNodeFlags_DefaultOpen, "Transform"))
			{
				auto& transform = entity.GetComponent<TransformComponent>();
				ImGui::DragFloat3("Position", glm::value_ptr(transform.Translation), 0.1f);
				ImGui::DragFloat3("Rotation", glm::value_ptr(transform.Rotation), 0.1f);
				ImGui::DragFloat3("Scale", glm::value_ptr(transform.Scale), 0.1f);

				ImGui::TreePop();
			}
		}
		if (entity.HasComponent<CameraComponent>())
		{
			if (ImGui::TreeNodeEx(reinterpret_cast<void*>(typeid(CameraComponent).hash_code()), ImGuiTreeNodeFlags_DefaultOpen, "Camera"))
			{
				auto& cameraComponent = entity.GetComponent<CameraComponent>();
				auto& camera = cameraComponent.Camera;

				const char* projectionTypeStrings[] = { "Perspective", "Orthographic" };
				const char* currentProjectionTypeString = projectionTypeStrings[static_cast<int>(camera.GetProjectionType())];

				if (ImGui::BeginCombo("Projection Type", currentProjectionTypeString))
				{
					for (int i = 0; i < 2; i++)
					{
						const bool isSelected = (currentProjectionTypeString == projectionTypeStrings[i]);
						if (ImGui::Selectable(projectionTypeStrings[i], isSelected))
						{
							camera.SetProjectionType(static_cast<SceneCamera::ProjectionType>(i));
						}
						if (isSelected)
						{
							ImGui::SetItemDefaultFocus();
						}
					}

					ImGui::EndCombo();
				}

				if (camera.GetProjectionType() == SceneCamera::ProjectionType::Orthographic)
				{
					float size = camera.GetOrthographicSize();
					if (ImGui::DragFloat("Size", &size))
					{
						camera.SetOrthographicSize(size);
					}
					float nearClip = camera.GetOrthographicNearClip();
					if (ImGui::DragFloat("Near Clip", &nearClip))
					{
						camera.SetOrthographicNearClip(nearClip);
					}
					float farClip = camera.GetOrthographicFarClip();
					if (ImGui::DragFloat("Far Clip", &farClip))
					{
						camera.SetOrthographicFarClip(farClip);
					}
				}
				else
				{
					float fov = glm::degrees(camera.GetPerspectiveFov());
					if (ImGui::DragFloat("FOV", &fov))
					{
						camera.SetPerspectiveFov(glm::radians(fov));
					}
					float nearClip = camera.GetPerspectiveNearClip();
					if (ImGui::DragFloat("Near Clip", &nearClip))
					{
						camera.SetPerspectiveNearClip(nearClip);
					}
					float farClip = camera.GetPerspectiveFarClip();
					if (ImGui::DragFloat("Far Clip", &farClip))
					{
						camera.SetPerspectiveFarClip(farClip);
					}
				}

				ImGui::Checkbox("Primary", &cameraComponent.IsPrimary);
				ImGui::Checkbox("Fixed Aspect Ratio", &cameraComponent.FixedAspectRatio);

				ImGui::TreePop();
			}
		}	
		if (entity.HasComponent<SpriteRendererComponent>())
		{
			if (ImGui::TreeNodeEx(reinterpret_cast<void*>(typeid(SpriteRendererComponent).hash_code()), ImGuiTreeNodeFlags_DefaultOpen, "Sprite"))
			{
				auto& spriteRenderer = entity.GetComponent<SpriteRendererComponent>();
				ImGui::ColorEdit4("Color", &spriteRenderer.Color[0]);

				ImGui::TreePop();
			}
		}
	}
}
