#include "HierarchyPanel.h"
#include "Kerberos/Scene/Components.h"
#include "Kerberos/Assets/AssetManager.h"
#include "Kerberos/Assets/Importers/TextureImporter.h"

#include <glm/gtc/type_ptr.hpp>
#include <imgui/imgui.h>
#include "imgui/imgui_internal.h"
#include <filesystem>

namespace Kerberos
{
	HierarchyPanel::HierarchyPanel(const Ref<Scene>& context)
		: m_Context(context)
	{
		SetContext(context);
	}

	void HierarchyPanel::SetContext(const Ref<Scene>& context)
	{
		m_Context = context;

		/// This is neccessary to avoid having a dangling pointer to the previous context
		/// when loading a new scene
		m_SelectedEntity = {};

		m_IceTexture = TextureImporter::ImportTexture("assets/textures/y2k_ice_texture.png");
		m_SpriteSheetTexture = TextureImporter::ImportTexture("assets/game/textures/RPGpack_sheet_2X.png");
		m_CubeMesh = Mesh::CreateCube(1.0f);
		m_SphereMesh = Mesh::CreateSphere(1.0f, 16, 16);
		m_WhiteMaterial = CreateRef<Material>();

		m_WhiteTexture = AssetManager::GetDefaultTexture2D();

		FramebufferSpecification frameBufferSpec;
		frameBufferSpec.Width = 256;
		frameBufferSpec.Height = 256;
		frameBufferSpec.Attachments = { FramebufferTextureFormat::RGBA8, FramebufferTextureFormat::Depth };
		m_CubemapFramebuffer = Framebuffer::Create(frameBufferSpec);
		m_CubemapFramebuffer->SetDebugName("Depth Map Visualization");
	}

	void HierarchyPanel::OnImGuiRender()
	{
		ImGui::Begin("Hierarchy");

		/// Only draw the root nodes
		for (const entt::entity& entityId : m_Context->GetRootEntities())
		{
			Entity rootEntity{ entityId, m_Context.get() };
			DrawEntityNode(rootEntity);
		}

		//for (const auto entityId : m_Context->m_Registry.view<entt::entity>())
		//{
		//	Entity e{ entityId, m_Context.get() };

		//	if (!m_Context->GetParent(e))
		//		DrawEntityNode(e);
		//}

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

				if (ImGui::MenuItem("Static Mesh"))
				{
					m_SelectedEntity.AddComponent<StaticMeshComponent>();
					ImGui::CloseCurrentPopup();
				}

				if (ImGui::MenuItem("Rigidbody3D"))
				{
					m_SelectedEntity.AddComponent<RigidBody3DComponent>();
					ImGui::CloseCurrentPopup();
				}

				if (ImGui::MenuItem("Box Collider 3D"))
				{
					m_SelectedEntity.AddComponent<BoxCollider3DComponent>();
					ImGui::CloseCurrentPopup();
				}

				if (ImGui::MenuItem("Environment"))
				{
					m_SelectedEntity.AddComponent<EnvironmentComponent>();
					ImGui::CloseCurrentPopup();
				}

				ImGui::EndPopup();
			}
		}
		ImGui::End();

		for (const auto& e : m_DeletionQueue)
		{
			m_Context->DestroyEntity(e);
		}
		m_DeletionQueue.clear();

		m_NotificationManager.RenderNotifications();
	}

	void HierarchyPanel::SetSelectedEntity(const Entity entity)
	{
		m_SelectedEntity = entity;
	}

	void HierarchyPanel::DrawEntityNode(const Entity& entity)
	{
		const auto& tag = entity.GetComponent<TagComponent>().Tag;

		const ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth
			| (entity == m_SelectedEntity ? ImGuiTreeNodeFlags_Selected : 0);

		/// The entity's identifier serves as the unique ID for the ImGui tree node
		const bool opened = ImGui::TreeNodeEx(reinterpret_cast<void*>(static_cast<uint64_t>(entity.GetUUID())), flags, "%s", tag.c_str());

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
			for (const Entity& child : m_Context->GetChildren(entity))
			{
				DrawEntityNode(child);
			}

			ImGui::TreePop();
		}

		if (entityDeleted)
		{
			m_DeletionQueue.push_back(entity);
			if (m_SelectedEntity == entity)
			{
				m_SelectedEntity = {};
			}
		}
	}

	static void DrawVec3Control(const std::string& label, glm::vec3& values, const float resetValue = 0.0f, const float columnWidth = 80.0f, const std::function<void()>& onValueChanged = nullptr)
	{
		const ImGuiIO& io = ImGui::GetIO();
		const auto boldFont = io.Fonts->Fonts[1];

		ImGui::PushID(label.c_str());

		ImGui::Columns(2);
		ImGui::SetColumnWidth(0, columnWidth);
		ImGui::Text("%s", label.c_str());
		ImGui::NextColumn();

		ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));

		const float lineHeight = ImGui::GetFontSize() + ImGui::GetStyle().FramePadding.y * 2.0f;
		const ImVec2 buttonSize = ImVec2(lineHeight + 2.0f, lineHeight);

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.2f, 0.2f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.9f, 0.25f, 0.25f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.8f, 0.2f, 0.25f, 1.0f));

		ImGui::PushFont(boldFont);
		if (ImGui::Button("X", buttonSize))
		{
			values.x = resetValue;
			if (onValueChanged)
				onValueChanged();
		}
		ImGui::PopFont();

		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		/// ##X is used to hide the label of the input field
		if (ImGui::DragFloat("##X", &values.x, 0.1f))
		{
			if (onValueChanged)
				onValueChanged();
		}

		ImGui::PopItemWidth();
		ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.8f, 0.2f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.25f, 0.9f, 0.25f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.25f, 0.8f, 0.2f, 1.0f));

		ImGui::PushFont(boldFont);
		if (ImGui::Button("Y", buttonSize))
		{
			values.y = resetValue;
			if (onValueChanged)
				onValueChanged();
		}
		ImGui::PopFont();

		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		/// ##Y is used to hide the label of the input field
		if (ImGui::DragFloat("##Y", &values.y, 0.1f))
		{
			if (onValueChanged)
				onValueChanged();
		}

		ImGui::PopItemWidth();
		ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.2f, 0.8f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.25f, 0.25f, 0.9f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.2f, 0.25f, 0.8f, 1.0f));

		ImGui::PushFont(boldFont);
		if (ImGui::Button("Z", buttonSize))
		{
			values.z = resetValue;
			if (onValueChanged)
				onValueChanged();
		}
		ImGui::PopFont();

		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		/// ##Z is used to hide the label of the input field
		if (ImGui::DragFloat("##Z", &values.z, 0.1f))
		{
			if (onValueChanged)
				onValueChanged();
		}
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
				const auto onValueChanged = [&entity, this]()
					{
						m_Context->CalculateEntityTransform(entity);
					};

				DrawVec3Control("Position", transform.Translation, 0.0f, 80.0f, onValueChanged);
				DrawVec3Control("Rotation", transform.Rotation, 0.0f, 80.0f, onValueChanged);
				DrawVec3Control("Scale", transform.Scale, 1.0f, 80.f, onValueChanged);

				ImGui::TreePop();
			}
		}

		constexpr ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_AllowItemOverlap;

		if (entity.HasComponent<CameraComponent>())
		{
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(6, 6));
			const bool opened = ImGui::TreeNodeEx(reinterpret_cast<void*>(typeid(CameraComponent).hash_code()), treeNodeFlags, "Camera");
			ImGui::SameLine(ImGui::GetContentRegionAvail().x - 20.0f);
			if (ImGui::Button("+", ImVec2{ 20, 20 }))
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
				ImGui::ColorEdit3("Color", &directionalLight.Light.Color[0]);
				if (ImGui::DragFloat("Intensity", &directionalLight.Light.Intensity, 0.01f, 0.0f, 10.0f))
				{
					directionalLight.NeedsUpdate = true;
				}
				DrawVec3Control("Direction", directionalLight.Light.Direction, 0.0f, 80.0f, [&directionalLight]()
					{
						directionalLight.NeedsUpdate = true;
					});
				ImGui::Checkbox("Enabled", &directionalLight.IsEnabled);
				ImGui::Checkbox("Cast Shadows", &directionalLight.CastShadows);

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
				ImGui::Checkbox("Enabled", &pointLight.IsEnabled);
				ImGui::ColorEdit3("Color", &pointLight.Light.Color[0]);
				DrawVec3Control("Position", pointLight.Light.Position);
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
		if (entity.HasComponent<SpotLightComponent>())
		{
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(6, 6));
			const bool opened = ImGui::TreeNodeEx(reinterpret_cast<void*>(typeid(SpotLightComponent).hash_code()), treeNodeFlags, "Spotlight");
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
				auto& spotlight = entity.GetComponent<SpotLightComponent>();
				ImGui::ColorEdit3("Color", &spotlight.Light.Color[0]);
				ImGui::DragFloat("Intensity", &spotlight.Light.Intensity, 0.01f, 0.0f, 10.0f);
				DrawVec3Control("Position", spotlight.Light.Position);
				ImGui::Checkbox("Enabled", &spotlight.IsEnabled);

				ImGui::TreePop();
			}

			if (componentDeleted)
			{
				entity.RemoveComponent<SpotLightComponent>();
			}
		}
		if (entity.HasComponent<StaticMeshComponent>())
		{
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(6, 6));
			const bool opened = ImGui::TreeNodeEx(reinterpret_cast<void*>(typeid(StaticMeshComponent).hash_code()), treeNodeFlags, "Static Mesh");
			ImGui::SameLine(ImGui::GetContentRegionAvail().x - 20.f);
			if (ImGui::Button("+", ImVec2{ 20, 20 }))
			{
				ImGui::OpenPopup("ComponentSettings");
			}
			ImGui::PopStyleVar();

			bool componentDeleted = false;
			if (ImGui::BeginPopup("ComponentSettings"))
			{
				ImGui::Text("Static Mesh Settings");
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
				auto& staticMesh = entity.GetComponent<StaticMeshComponent>();
				ImGui::Checkbox("Visible", &staticMesh.Visible);

				/// TODO: Visualize the static mesh in the editor
				ImGui::Button("Static Mesh");

				ImGui::Separator();

				ImGui::Checkbox("Cast Shadows", &staticMesh.CastShadows);

				ImGui::Separator();

				/// Handle drag and drop for meshes
				if (ImGui::BeginDragDropTarget())
				{
					if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_BROWSER_MESH"))
					{
						const AssetHandle handle = *static_cast<AssetHandle*>(payload->Data);
						if (AssetManager::GetAssetType(handle) != AssetType::Mesh)
						{
							KBR_ERROR("Asset is not a mesh: {0}", handle);
							m_NotificationManager.AddNotification("Asset is not a mesh", Notification::Type::Error);
							return;
						}
						const Ref<Mesh> mesh = AssetManager::GetAsset<Mesh>(handle);
						staticMesh.StaticMesh = mesh;
					}
					ImGui::EndDragDropTarget();
				}

				if (staticMesh.MeshMaterial)
				{
					ImGui::Separator();
					ImGui::Text("Material");
					ImGui::ColorEdit3("Diffuse", &staticMesh.MeshMaterial->Diffuse[0]);
					ImGui::ColorEdit3("Ambient", &staticMesh.MeshMaterial->Ambient[0]);
					ImGui::ColorEdit3("Specular", &staticMesh.MeshMaterial->Specular[0]);
					ImGui::DragFloat("Shininess", &staticMesh.MeshMaterial->Shininess, 0.1f, 0.0f, 10.0f);
				}

				ImGui::Separator();
				ImGui::Text("Texture");
				uint64_t textureID;
				if (staticMesh.MeshTexture)
				{
					textureID = staticMesh.MeshTexture->GetRendererID();
				}
				else
				{
					textureID = m_WhiteTexture->GetRendererID();
				}
				ImGui::Image(textureID, ImVec2{ 64, 64 }, ImVec2{ 0, 1 }, ImVec2{ 1, 0 });

				/// Handle drag and drop for textures
				if (ImGui::BeginDragDropTarget())
				{
					if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_BROWSER_TEXTURE"))
					{
						const AssetHandle handle = *static_cast<AssetHandle*>(payload->Data);
						if (AssetManager::GetAssetType(handle) != AssetType::Texture2D)
						{
							KBR_ERROR("Asset is not a texture: {0}", handle);
							m_NotificationManager.AddNotification("Asset is not a texture", Notification::Type::Error);
							return;
						}
						const Ref<Texture2D> texture = AssetManager::GetAsset<Texture2D>(handle);
						staticMesh.MeshTexture = texture;
					}
					ImGui::EndDragDropTarget();
				}

				/// Clear the texture if needed
				if (ImGui::BeginPopupContextItem("Texture Options"))
				{
					ImGui::TextDisabled("%s", "Texture Options");
					ImGui::Separator();
					if (ImGui::MenuItem("Clear Texture"))
					{
						staticMesh.MeshTexture = m_WhiteTexture;
						ImGui::CloseCurrentPopup();
					}
					ImGui::EndPopup();
				}

				ImGui::TreePop();
			}

			if (componentDeleted)
			{
				entity.RemoveComponent<StaticMeshComponent>();
			}
		}
		if (entity.HasComponent<RigidBody3DComponent>())
		{
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(6, 6));
			const bool opened = ImGui::TreeNodeEx(reinterpret_cast<void*>(typeid(RigidBody3DComponent).hash_code()), treeNodeFlags, "RigidBody3D");
			ImGui::SameLine(ImGui::GetContentRegionAvail().x - 20.f);
			if (ImGui::Button("+", ImVec2{ 20, 20 }))
			{
				ImGui::OpenPopup("ComponentSettings");
			}
			ImGui::PopStyleVar();

			bool componentDeleted = false;
			if (ImGui::BeginPopup("ComponentSettings"))
			{
				ImGui::Text("RigidBody3D Settings");
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
				auto& rb = entity.GetComponent<RigidBody3DComponent>();

				const char* bodyTypeStrings[] = { "Static", "Dynamic", "Kinematic" };
				const char* currentBodyTypeString = bodyTypeStrings[static_cast<int>(rb.Type)];
				if (ImGui::BeginCombo("Body Type", currentBodyTypeString))
				{
					for (int i = 0; i < 3; i++)
					{
						const bool isSelected = (currentBodyTypeString == bodyTypeStrings[i]);
						if (ImGui::Selectable(bodyTypeStrings[i], isSelected))
						{
							rb.Type = static_cast<RigidBody3DComponent::BodyType>(i);
						}
						if (isSelected)
						{
							ImGui::SetItemDefaultFocus();
						}
					}
					ImGui::EndCombo();
				}

				ImGui::DragFloat("Friction", &rb.Friction, 0.02f, 0.0f, 1.0f);
				ImGui::DragFloat("Restitution", &rb.Restitution, 0.02f, 0.0f, 1.0f);
				ImGui::DragFloat3("Velocity", glm::value_ptr(rb.Velocity), 0.1f);
				ImGui::DragFloat3("Angular Velocity", glm::value_ptr(rb.AngularVelocity));
				ImGui::Checkbox("Use Gravity", &rb.UseGravity);
				ImGui::DragFloat("Mass", &rb.Mass, 0.1f, 0.01f, 100.0f);

				ImGui::TreePop();
			}

			if (componentDeleted)
			{
				entity.RemoveComponent<RigidBody3DComponent>();
			}
		}
		if (entity.HasComponent<BoxCollider3DComponent>())
		{
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(6, 6));
			const bool opened = ImGui::TreeNodeEx(reinterpret_cast<void*>(typeid(BoxCollider3DComponent).hash_code()), treeNodeFlags, "BoxCollider3D");
			ImGui::SameLine(ImGui::GetContentRegionAvail().x - 20.f);
			if (ImGui::Button("+", ImVec2{ 20, 20 }))
			{
				ImGui::OpenPopup("ComponentSettings");
			}
			ImGui::PopStyleVar();

			bool componentDeleted = false;
			if (ImGui::BeginPopup("ComponentSettings"))
			{
				ImGui::Text("BoxCollider3D Settings");
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
				auto& collider = entity.GetComponent<BoxCollider3DComponent>();
				ImGui::DragFloat3("Size", glm::value_ptr(collider.Size), 0.1f);
				ImGui::DragFloat3("Offset", glm::value_ptr(collider.Offset), 0.1f);
				ImGui::Checkbox("Is Trigger", &collider.IsTrigger);

				ImGui::TreePop();
			}

			if (componentDeleted)
			{
				entity.RemoveComponent<BoxCollider3DComponent>();
			}
		}
		if (entity.HasComponent<EnvironmentComponent>())
		{
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(6, 6));
			const bool opened = ImGui::TreeNodeEx(reinterpret_cast<void*>(typeid(EnvironmentComponent).hash_code()), treeNodeFlags, "Environment");
			ImGui::SameLine(ImGui::GetContentRegionAvail().x - 20.f);
			if (ImGui::Button("+", ImVec2{ 20, 20 }))
			{
				ImGui::OpenPopup("ComponentSettings");
			}
			ImGui::PopStyleVar();
			bool componentDeleted = false;
			if (ImGui::BeginPopup("ComponentSettings"))
			{
				ImGui::Text("Environment Settings");
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
				auto& environment = entity.GetComponent<EnvironmentComponent>();
				ImGui::Checkbox("Skybox Enabled", &environment.IsSkyboxEnabled);

				/// Render the environment cubemap into an image
				ImGui::Text("Skybox Texture");
				const uint64_t textureID = m_CubemapFramebuffer->GetColorAttachmentRendererID();
				ImGui::Image(textureID, ImVec2{ 256, 256 }, ImVec2{ 0, 1 }, ImVec2{ 1, 0 });

				/// Handle drag and drop for textures
				if (ImGui::BeginDragDropTarget())
				{
					if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_BROWSER_TEXTURE_CUBE"))
					{
						const AssetHandle handle = *static_cast<AssetHandle*>(payload->Data);
						if (AssetManager::GetAssetType(handle) != AssetType::TextureCube)
						{
							KBR_ERROR("Asset is not a texture: {0}", handle);
							/// TODO: Show a notification instead of an error log
							m_NotificationManager.AddNotification("Asset is not a cubemap", Notification::Type::Error);
							return;
						}
						environment.SkyboxTexture = handle;

						/// render the changed cube into the environment
						/*m_CubemapFramebuffer->Bind();
						m_CubemapFramebuffer->Unbind();*/
					}
					ImGui::EndDragDropTarget();
				}

				ImGui::TreePop();
			}
			if (componentDeleted)
			{
				entity.RemoveComponent<EnvironmentComponent>();
			}
		}
	}
}
