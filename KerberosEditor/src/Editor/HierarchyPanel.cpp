#include "HierarchyPanel.h"

#include <glm/gtc/type_ptr.hpp>

#include "Kerberos/Scene/Components.h"
#include <imgui/imgui.h>

#include "imgui/imgui_internal.h"

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

		/// If an empty space is clicked, deselect the entity
		if (ImGui::IsMouseDown(0) && ImGui::IsWindowHovered())
		{
			m_SelectedEntity = {};
		}

		/// If the right mouse button is clicked, show the context menu
		if (ImGui::BeginPopupContextWindow(nullptr, 1 | ImGuiPopupFlags_NoOpenOverItems))
		{
			if (ImGui::MenuItem("Create Empty Entity"))
			{
				m_Context->CreateEntity("Empty Entity");
			}

			ImGui::EndPopup();
		}
		ImGui::End();

		ImGui::Begin("Properties");
		if (m_SelectedEntity)
		{
			DrawComponents(m_SelectedEntity);

			if (ImGui::Button("Add Component"))
			{
				ImGui::OpenPopup("Add Component");
			}

			if (ImGui::BeginPopup("Add Component"))
			{
				if (ImGui::MenuItem("Camera"))
				{
					m_SelectedEntity.AddComponent<CameraComponent>();
					ImGui::CloseCurrentPopup();
				}

				if (ImGui::MenuItem("Sprite Renderer"))
				{
					m_SelectedEntity.AddComponent<SpriteRendererComponent>();
					ImGui::CloseCurrentPopup();
				}

				if (ImGui::MenuItem("Directional Light"))
				{
					m_SelectedEntity.AddComponent<DirectionalLightComponent>();
					ImGui::CloseCurrentPopup();
				}

				if (ImGui::MenuItem("Point Light"))
				{
					m_SelectedEntity.AddComponent<PointLightComponent>();
					ImGui::CloseCurrentPopup();
				}

				/*if (ImGui::MenuItem("Box Collider"))
				{
					m_SelectedEntity.AddComponent<BoxCollider2DComponent>();
					ImGui::CloseCurrentPopup();
				}*/

				ImGui::EndPopup();
			}
		}
		ImGui::End();
	}

	void HierarchyPanel::DrawEntityNode(const Entity& entity)
	{
		const auto& tag = entity.GetComponent<TagComponent>().Tag;

		const ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth 
			| (entity == m_SelectedEntity ? ImGuiTreeNodeFlags_Selected : 0);

		/// The entity's identifier serves as the unique ID for the ImGui tree node
		const bool opened = ImGui::TreeNodeEx(reinterpret_cast<void*>(static_cast<uint64_t>(static_cast<uint32_t>(entity))), flags, "%s", tag.c_str());

		if (ImGui::IsItemClicked())
		{
			m_SelectedEntity = entity;
		}

		/// Defer the deletion of the entity until the popup is closed to avoid invalidating the iterator
		bool entityDeleted = false;
		if (ImGui::BeginPopupContextItem())
		{
			if (ImGui::MenuItem("Delete Entity"))
			{
				entityDeleted = true;
			}
			ImGui::EndPopup();
		}

		if (opened)
		{
			constexpr ImGuiTreeNodeFlags childFlags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
			if (bool childOpened = ImGui::TreeNodeEx(reinterpret_cast<void*>(9817239), childFlags, tag.c_str()))
			{
				ImGui::TreePop();
			}
			ImGui::TreePop();
		}

		if (entityDeleted)
		{
			m_Context->DestroyEntity(entity);
			if (m_SelectedEntity == entity)
			{
				m_SelectedEntity = {};
			}
		}
	}

	static void DrawVec3Control(const std::string& label, glm::vec3& values, const float resetValue = 0.0f, const float columnWidth = 80.0f)
	{
		ImGui::PushID(label.c_str());

		ImGui::Columns(2);
		ImGui::SetColumnWidth(0, columnWidth);
		ImGui::Text(label.c_str());
		ImGui::NextColumn();

		ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));

		const float lineHeight = ImGui::GetFontSize() + ImGui::GetStyle().FramePadding.y * 2.0f;
		const ImVec2 buttonSize = ImVec2(lineHeight + 2.0f, lineHeight);

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.2f, 0.2f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.9f, 0.25f, 0.25f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.8f, 0.2f, 0.25f, 1.0f));

		if (ImGui::Button("X", buttonSize))
		{
			values.x = resetValue;
		}

		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		/// ##X is used to hide the label of the input field
		ImGui::DragFloat("##X", &values.x, 0.1f);
		ImGui::PopItemWidth();
		ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.8f, 0.2f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.25f, 0.9f, 0.25f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.25f, 0.8f, 0.2f, 1.0f));

		if (ImGui::Button("Y", buttonSize))
		{
			values.y = resetValue;
		}

		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		/// ##Y is used to hide the label of the input field
		ImGui::DragFloat("##Y", &values.y, 0.1f);
		ImGui::PopItemWidth();
		ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.2f, 0.8f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.25f, 0.25f, 0.9f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.2f, 0.25f, 0.8f, 1.0f));

		if (ImGui::Button("Z", buttonSize))
		{
			values.z = resetValue;
		}

		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		/// ##Z is used to hide the label of the input field
		ImGui::DragFloat("##Z", &values.z, 0.1f);
		ImGui::PopItemWidth();

		ImGui::PopStyleVar();

		/// Reset the column value to be the default
		ImGui::Columns(1);

		ImGui::PopID();
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
				DrawVec3Control("Position", transform.Translation);
				DrawVec3Control("Rotation", transform.Rotation);
				DrawVec3Control("Scale", transform.Scale, 1.0f);

				ImGui::TreePop();
			}
		}

		constexpr ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_AllowItemOverlap;

		if (entity.HasComponent<CameraComponent>())
		{
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(6, 6));
			const bool opened = ImGui::TreeNodeEx(reinterpret_cast<void*>(typeid(CameraComponent).hash_code()), treeNodeFlags, "Camera");
			ImGui::SameLine(ImGui::GetContentRegionAvail().x - 20.0f);
			if (ImGui::Button("+", ImVec2{20, 20}))
			{
				ImGui::OpenPopup("ComponentSettings");
			}
			ImGui::PopStyleVar();

			bool componentDeleted = false;
			if (ImGui::BeginPopup("ComponentSettings"))
			{
				ImGui::Text("Camera Settings");
				ImGui::Separator();
				if (ImGui::MenuItem("Remove Component"))
				{
					componentDeleted = true;
					ImGui::CloseCurrentPopup();
				}
				ImGui::EndPopup();
			}

			if (opened)
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

			if (componentDeleted)
			{
				entity.RemoveComponent<CameraComponent>();
			}
		}	
		if (entity.HasComponent<SpriteRendererComponent>())
		{
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(6, 6));
			const bool opened = ImGui::TreeNodeEx(reinterpret_cast<void*>(typeid(SpriteRendererComponent).hash_code()), treeNodeFlags, "Sprite");
			ImGui::SameLine(ImGui::GetContentRegionAvail().x - 20.f);
			if (ImGui::Button("+", ImVec2{ 20, 20 }))
			{
				ImGui::OpenPopup("ComponentSettings");
			}
			ImGui::PopStyleVar();

			bool componentDeleted = false;
			if (ImGui::BeginPopup("ComponentSettings"))
			{
				ImGui::Text("Sprite Renderer Settings");
				ImGui::Separator();
				if (ImGui::MenuItem("Remove Component"))
				{
					componentDeleted = true;
					ImGui::CloseCurrentPopup();
				}
				ImGui::EndPopup();
			}

			if (opened)
			{
				auto& spriteRenderer = entity.GetComponent<SpriteRendererComponent>();
				ImGui::ColorEdit4("Color", &spriteRenderer.Color[0]);

				ImGui::TreePop();
			}

			if (componentDeleted)
			{
				entity.RemoveComponent<SpriteRendererComponent>();
			}
		}
		if (entity.HasComponent<DirectionalLightComponent>())
		{
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(6, 6));
			const bool opened = ImGui::TreeNodeEx(reinterpret_cast<void*>(typeid(DirectionalLightComponent).hash_code()), treeNodeFlags, "Directional Light");
			ImGui::SameLine(ImGui::GetContentRegionAvail().x - 20.f);
			if (ImGui::Button("+", ImVec2{ 20, 20 }))
			{
				ImGui::OpenPopup("ComponentSettings");
			}
			ImGui::PopStyleVar();

			bool componentDeleted = false;
			if (ImGui::BeginPopup("ComponentSettings"))
			{
				ImGui::Text("Directional Light Settings");
				ImGui::Separator();
				if (ImGui::MenuItem("Remove Component"))
				{
					componentDeleted = true;
					ImGui::CloseCurrentPopup();
				}
				ImGui::EndPopup();
			}

			if (opened)
			{
				auto& directionalLight = entity.GetComponent<DirectionalLightComponent>();
				ImGui::ColorEdit4("Color", &directionalLight.Light.Color[0]);
				ImGui::DragFloat("Intensity", &directionalLight.Light.Intensity, 0.01f, 0.0f, 10.0f);
				ImGui::ColorEdit3("Direction", &directionalLight.Light.Direction[0]);

				ImGui::TreePop();
			}

			if (componentDeleted)
			{
				entity.RemoveComponent<DirectionalLightComponent>();
			}
		}
		if (entity.HasComponent<PointLightComponent>())
		{
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(6, 6));
			const bool opened = ImGui::TreeNodeEx(reinterpret_cast<void*>(typeid(PointLightComponent).hash_code()), treeNodeFlags, "Point Light");
			ImGui::SameLine(ImGui::GetContentRegionAvail().x - 20.f);
			if (ImGui::Button("+", ImVec2{ 20, 20 }))
			{
				ImGui::OpenPopup("ComponentSettings");
			}
			ImGui::PopStyleVar();

			bool componentDeleted = false;
			if (ImGui::BeginPopup("ComponentSettings"))
			{
				ImGui::Text("Point Light Settings");
				ImGui::Separator();
				if (ImGui::MenuItem("Remove Component"))
				{
					componentDeleted = true;
					ImGui::CloseCurrentPopup();
				}
				ImGui::EndPopup();
			}

			if (opened)
			{
				auto& pointLight = entity.GetComponent<PointLightComponent>();
				ImGui::ColorEdit4("Color", &pointLight.Light.Color[0]);
				ImGui::ColorEdit3("Position", &pointLight.Light.Position[0]);
				ImGui::DragFloat("Intensity", &pointLight.Light.Intensity, 0.01f, 0.0f, 10.0f);
				ImGui::Text("Attenuation factors");
				ImGui::DragFloat("Constant", &pointLight.Light.Constant, 0.01f, 0.0f, 1.0f);
				ImGui::DragFloat("Linear", &pointLight.Light.Linear, 0.01f, 0.0f, 1.0f);
				ImGui::DragFloat("Quadratic", &pointLight.Light.Quadratic, 0.01f, 0.0f, 1.0f);

				ImGui::TreePop();
			}

			if (componentDeleted)
			{
				entity.RemoveComponent<PointLightComponent>();
			}
		}
	}
}
