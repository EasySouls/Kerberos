#include "EditorLayer.h"

#include "Kerberos/Utils/PlatformUtils.h"

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/type_ptr.inl>

#include "imgui/imgui.h"
#include <ImGuizmo/ImGuizmo.h>

#include "Kerberos/Assets/Importers/TextureImporter.h"

#define PROFILE_SCOPE(name) Timer timer##__LINE__(name, 

namespace Kerberos
{
	EditorLayer::EditorLayer()
		: Layer("EditorLayer"), m_CameraController(1280.0f / 720.0f)
	{}

	void EditorLayer::OnAttach()
	{
		KBR_PROFILE_FUNCTION();

		m_ActiveScene = CreateRef<Scene>();

		/// TODO: Open the project passed as command line argument, if there is one
#define TESTING 1
#if TESTING
		OpenProject(R"(C:\Development\Kerberos\KerberosEditor\World3D.kbrproj)");
#else
		if (!OpenProject())
		{
			NewProject();
		}
#endif

		m_EditorCamera = EditorCamera(45.0f, 16.0f / 9.0f, 0.1f, 1000.0f);

		m_ViewportSize = { 1280.0f, 720.0f };

		m_Texture = TextureImporter::ImportTexture("assets/textures/y2k_ice_texture.png");

		m_SpriteSheet = TextureImporter::ImportTexture("assets/game/textures/RPGpack_sheet_2X.png");

		m_TextureStairs = SubTexture2D::CreateFromCoords(m_SpriteSheet, { 7, 6 }, { 128, 128 }, { 1, 1 });
		m_TextureBarrel = SubTexture2D::CreateFromCoords(m_SpriteSheet, { 8, 3 }, { 128, 128 }, { 1, 1 });
		m_TextureTree = SubTexture2D::CreateFromCoords(m_SpriteSheet, { 2, 1 }, { 128, 128 }, { 1, 2 });

		m_TextureGrass = SubTexture2D::CreateFromCoords(m_SpriteSheet, { 1, 11 }, { 128, 128 }, { 1, 1 });
		m_TextureDirt = SubTexture2D::CreateFromCoords(m_SpriteSheet, { 6, 11 }, { 128, 128 }, { 1, 1 });
		m_TextureWater = SubTexture2D::CreateFromCoords(m_SpriteSheet, { 11, 11 }, { 128, 128 }, { 1, 1 });

#define FIRST_TIME_LOADING_SCENE 1
#if FIRST_TIME_LOADING_SCENE
		Entity squareEntity = m_ActiveScene->CreateEntity("Square");
		squareEntity.AddComponent<SpriteRendererComponent>(glm::vec4{ 0.2f, 0.3f, 0.8f, 1.0f });

		const Ref<Material> whiteMaterial = CreateRef<Material>();

		{
			Entity cubeEntity = m_ActiveScene->CreateEntity("Cube");
			const Ref<Mesh> cubeMesh = Mesh::CreateCube(1.0f);
			cubeEntity.AddComponent<StaticMeshComponent>(cubeMesh, whiteMaterial, m_Texture);
			auto& transform = cubeEntity.GetComponent<TransformComponent>();
			transform.Translation = { -2.0f, 2.0f, -2.0f };
			transform.Rotation = { 20, 10, 86 };
			cubeEntity.AddComponent<BoxCollider3DComponent>();
			cubeEntity.AddComponent<RigidBody3DComponent>();
		}

		{
			Entity sphereEntity = m_ActiveScene->CreateEntity("Sphere");
			const Ref<Mesh> sphereMesh = Mesh::CreateSphere(1.0f, 32, 32);
			sphereEntity.AddComponent<StaticMeshComponent>(sphereMesh, whiteMaterial, m_Texture);
			sphereEntity.GetComponent<TransformComponent>().Translation = { 2.0f, 1.2f, -2.0f };
		}

		//{
		//	Entity planeEntity = m_ActiveScene->CreateEntity("Plane");
		//	const Ref<Mesh> planeMesh = Mesh::CreatePlane(10.0f, 10.0f);
		//	planeEntity.AddComponent<StaticMeshComponent>(planeMesh, whiteMaterial, nullptr);
		//	planeEntity.GetComponent<TransformComponent>().Translation = { 0.0f, -1.0f, 0.0f };
		//	planeEntity.AddComponent<RigidBody3DComponent>().Type = RigidBody3DComponent::BodyType::Static;
		//	planeEntity.AddComponent<BoxCollider3DComponent>().Size = { 10.f, 0.1f, 10.f };
		//}

		{
			Entity groundEntity = m_ActiveScene->CreateEntity("Ground");
			const Ref<Mesh> groundMesh = Mesh::CreateCube(1.0f);
			groundEntity.AddComponent<StaticMeshComponent>(groundMesh, whiteMaterial, nullptr);
			auto& transform = groundEntity.GetComponent<TransformComponent>();
			transform.Translation = { 0.0f, -2.0f, 0.0f };
			transform.Scale = { 25.0f, 0.2f, 25.0f };
			groundEntity.AddComponent<RigidBody3DComponent>().Type = RigidBody3DComponent::BodyType::Static;
			groundEntity.AddComponent<BoxCollider3DComponent>().Size = { 25.0f, 0.2f, 25.f };
		}

		{
			Entity environmentEntity = m_ActiveScene->CreateEntity("Environment");
			environmentEntity.AddComponent<EnvironmentComponent>();
		}

		m_SunlightEntity = m_ActiveScene->CreateEntity("Sun");
		auto& sunlightComponent = m_SunlightEntity.AddComponent<DirectionalLightComponent>();
		sunlightComponent.Light.Color = { 1.0f, 1.0f, 0.8f };
		sunlightComponent.Light.Direction = { 123, -230, 130 };

		Entity pointLightEntity = m_ActiveScene->CreateEntity("Point Light");
		auto& pointLightComponent = pointLightEntity.AddComponent<PointLightComponent>();
		pointLightComponent.Light.Color = { 0.8f, 0.2f, 0.2f };
		pointLightComponent.Light.Position = { 0.9f, 4.1f, 3.9f };
		pointLightComponent.IsEnabled = false;

		m_CameraEntity = m_ActiveScene->CreateEntity("Camera");
		auto& cameraComponent = m_CameraEntity.AddComponent<CameraComponent>();
		cameraComponent.Camera.SetProjectionType(SceneCamera::ProjectionType::Perspective);
		auto& cameraTransformComponent = m_CameraEntity.GetComponent<TransformComponent>();
		cameraTransformComponent.Translation = { 0.0f, 0.0f, 5.0f };
#endif

		class CameraController : public ScriptableEntity
		{
		public:
			void OnUpdate(const Timestep ts) override
			{
				auto& translation = GetComponent<TransformComponent>().Translation;
				constexpr float speed = 5.0f;

				if (Input::IsKeyPressed(Key::W)) // w
					translation.y += speed * ts;
				if (Input::IsKeyPressed(Key::A)) // A
					translation.x -= speed * ts;
				if (Input::IsKeyPressed(Key::S)) // S
					translation.y -= speed * ts;
				if (Input::IsKeyPressed(Key::D)) // D
					translation.x += speed * ts;
			}
		};

		m_CameraEntity = m_ActiveScene->GetPrimaryCameraEntity();
		m_CameraEntity.AddComponent<NativeScriptComponent>().Bind<CameraController>();

		m_HierarchyPanel.SetContext(m_ActiveScene);

		//const Model backpackModel = Model("assets/models/backpack/backpack.obj", "Backpack");
		//backpackModel.InitEntities(m_ActiveScene);

		/*Model deerModel = Model("assets/models/deer_demo/scene.gltf", "Deer");
		deerModel.InitEntities(m_ActiveScene);*/

		m_IconPlay = TextureImporter::ImportTexture("assets/editor/play_button.png");
		m_IconStop = TextureImporter::ImportTexture("assets/editor/stop_button.png");

		/// Calculate the world transforms of the entities initially
		m_ActiveScene->CalculateEntityTransforms();
	}

	void EditorLayer::OnDetach()
	{
		Layer::OnDetach();
	}

	void EditorLayer::OnUpdate(const Timestep deltaTime)
	{
		KBR_PROFILE_FUNCTION();

		m_Fps = static_cast<float>(1) / deltaTime;

		Timer timer("EditorLayer::OnUpdate", [&](const ProfileResult profileResult) { m_ProfileResults.push_back(profileResult); });

		/// Resize the camera if needed
		if (const FramebufferSpecification spec = m_ActiveScene->GetEditorFramebuffer()->GetSpecification();
			m_ViewportSize.x > 0.0f && m_ViewportSize.y > 0.0f &&
			(spec.Width != static_cast<uint32_t>(m_ViewportSize.x) || spec.Height != static_cast<uint32_t>(m_ViewportSize.y)))
		{
			m_ActiveScene->GetEditorFramebuffer()->Resize(static_cast<uint32_t>(m_ViewportSize.x), static_cast<uint32_t>(m_ViewportSize.y));
			//m_CameraController.OnResize(m_ViewportSize.x, m_ViewportSize.y);
			m_EditorCamera.SetViewportSize(m_ViewportSize.x, m_ViewportSize.y);

			m_ActiveScene->OnViewportResize(static_cast<uint32_t>(m_ViewportSize.x), static_cast<uint32_t>(m_ViewportSize.y));
		}

		{
			KBR_PROFILE_SCOPE("EditorCamera::OnUpdate");

			switch (m_SceneState)
			{
			case SceneState::Edit:
			case SceneState::Simulate:
				m_EditorCamera.OnUpdate(deltaTime);
				break;
			case SceneState::Play:
			{
				/// Only update the camera when the viewport is focused
				if (m_ViewportFocused)
					m_CameraController.OnUpdate(deltaTime);
				break;
			}
			}
		}

		Renderer3D::ResetStatistics();

		{
			KBR_PROFILE_SCOPE("Renderer Prep");

			//m_ActiveScene->GetEditorFramebuffer()->Bind();

			//RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1 });
			//RenderCommand::Clear();

			///// Clear our entity ID attachment to -1, so when rendering entities they fill that space with their entity ID,
			///// and empty spacces will have -1, signaling that there is no entity.
			///// Used for mouse picking.
			//m_ActiveScene->GetEditorFramebuffer()->ClearAttachment(1, -1);
		}

		{
			KBR_PROFILE_SCOPE("Scene::OnUpdate");

			switch (m_SceneState)
			{
			case SceneState::Edit:
			case SceneState::Simulate:
				m_ActiveScene->OnUpdateEditor(deltaTime, m_EditorCamera);
				break;
			case SceneState::Play:
				m_ActiveScene->OnUpdateRuntime(deltaTime);
				break;
			}
		}

		{
			auto [mx, my] = ImGui::GetMousePos();
			mx -= m_ViewportBounds[0].x;
			my -= m_ViewportBounds[0].y;
			const glm::vec2 viewportSize = m_ViewportBounds[1] - m_ViewportBounds[0];

			/// Flip the y coord (works with opengl)
			my = viewportSize.y - my;

			const int mouseX = static_cast<int>(mx);
			const int mouseY = static_cast<int>(my);

			if (mouseX >= 0 && mouseY >= 0 && mouseX <= static_cast<int>(viewportSize.x) && mouseY <= static_cast<int>(viewportSize.y))
			{
				int pixelData = m_ActiveScene->GetEditorFramebuffer()->ReadPixel(1, mouseX, mouseY);

				if (pixelData < 0)
				{
					m_HoveredEntity = {};
				}
				else
				{
					m_HoveredEntity = Entity{ static_cast<entt::entity>(pixelData), m_ActiveScene.get() };
				}
			}
		}

		m_ActiveScene->GetEditorFramebuffer()->Unbind();
	}

	void EditorLayer::OnImGuiRender()
	{
		KBR_PROFILE_FUNCTION();

		static bool dockspaceOpen = true;
		static bool optFullscreenPersistent = true;
		static ImGuiDockNodeFlags dockspaceFlags = ImGuiDockNodeFlags_None;

		ImGuiWindowFlags windowFlags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;

		if (optFullscreenPersistent)
		{
			const ImGuiViewport* viewport = ImGui::GetMainViewport();
			ImGui::SetNextWindowPos(viewport->Pos);
			ImGui::SetNextWindowSize(viewport->Size);
			ImGui::SetNextWindowViewport(viewport->ID);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
			windowFlags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
			windowFlags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
		}

		if (dockspaceFlags & ImGuiDockNodeFlags_PassthruCentralNode)
			windowFlags |= ImGuiWindowFlags_NoBackground;

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::Begin("DockSpace Demo", &dockspaceOpen, windowFlags);
		ImGui::PopStyleVar();

		if (optFullscreenPersistent)
			ImGui::PopStyleVar(2);

		const ImGuiIO& io = ImGui::GetIO();
		if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
		{
			const ImGuiID dockspaceId = ImGui::GetID("MyDockSpace");
			ImGui::DockSpace(dockspaceId, ImVec2(0.0f, 0.0f), dockspaceFlags);
		}

		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("Exit"))
					Application::Get().Close();

				ImGui::MenuItem("Fullscreen", nullptr, &optFullscreenPersistent);

				if (ImGui::MenuItem("New Scene", "Ctrl+N"))
				{
					NewScene();
				}

				if (ImGui::MenuItem("Save", "Ctrl+S"))
				{
					SaveScene();
				}

				if (ImGui::MenuItem("Save As...", "Ctrl+Shift+S"))
				{
					SaveSceneAs();
				}

				if (ImGui::MenuItem("Load...", "Ctrl+O"))
				{
					LoadScene();
				}

				ImGui::EndMenu();
			}
			ImGui::EndMenuBar();
		}

		m_HierarchyPanel.OnImGuiRender();
		m_AssetsPanel->OnImGuiRender();

		ImGui::Begin("Settings");
		ImGui::ColorEdit3("Square Color", glm::value_ptr(m_SquareColor));

		const auto [DrawCalls, DrawnMeshes, Vertices, Faces] = Renderer3D::GetStatistics();
		ImGui::Text("Renderer3D Stats");
		ImGui::Text("Draw Calls: %u", DrawCalls);
		ImGui::Text("Meshes: %u", DrawnMeshes);
		ImGui::Text("Vertices: %u", Vertices);
		ImGui::Text("Faces: %u", Faces);

		for (const auto& [Name, Time] : m_ProfileResults)
		{
			const auto fmt = "%s %.3fms";
			ImGui::Text(fmt, Name, Time);
		}
		m_ProfileResults.clear();

		ImGui::Text("FPS: %.2f", m_Fps);

		if (ImGui::Checkbox("Toggle Primary Camera", &m_IsPrimaryCamera))
		{
			m_CameraEntity.GetComponent<CameraComponent>().IsPrimary = m_IsPrimaryCamera;
			m_SecondCamera.GetComponent<CameraComponent>().IsPrimary = !m_IsPrimaryCamera;
		}

		if (ImGui::Checkbox("Show Wireframe", &m_ShowWireframe))
		{
			Renderer3D::SetShowWireframe(m_ShowWireframe);
		}

		ImGui::Separator();

		ImGui::Text("Gizmo Type: %s", m_GizmoType == -1 ? "None" : (m_GizmoType == ImGuizmo::OPERATION::TRANSLATE ? "Translate" : (m_GizmoType == ImGuizmo::OPERATION::SCALE ? "Scale" : "Rotate")));
		ImGui::Text("Viewport Size: %.0f x %.0f", m_ViewportSize.x, m_ViewportSize.y);
		ImGui::Text("Viewport Focused: %s", m_ViewportFocused ? "Yes" : "No");
		ImGui::Text("Viewport Hovered: %s", m_ViewportHovered ? "Yes" : "No");

		std::string hoveredEntityName = "None";
		if (m_HoveredEntity)
		{
			hoveredEntityName = m_HoveredEntity.GetComponent<TagComponent>().Tag;
		}
		ImGui::Text("Hovered entity: %s", hoveredEntityName.c_str());

		ImGui::Separator();

		ImGui::Text("Editor Camera");
		const glm::vec3& cameraPosition = m_EditorCamera.GetPosition();
		ImGui::Text("Position: (%.2f, %.2f, %.2f)", cameraPosition.x, cameraPosition.y, cameraPosition.z);
		ImGui::Text("Rotation: (Pitch: %.2f, Yaw: %.2f)", m_EditorCamera.GetPitch(), m_EditorCamera.GetYaw());
		ImGui::Text("Distance: %.2f", m_EditorCamera.GetDistance());

		ImGui::Separator();

		/// Render the shadow map texture
		ImGui::Text("Shadow Map");
		const uint64_t shadowMapTextureID = m_ActiveScene->GetShadowMapFramebuffer()->GetDepthAttachmentRendererID();
		ImGui::Image(shadowMapTextureID, ImVec2{ 256, 256 }, ImVec2{ 0, 1 }, ImVec2{ 1, 0 });

		ImGui::End();

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::Begin("Viewport");
		ImGui::PopStyleVar();

		const auto viewportOffset = ImGui::GetCursorPos(); // Includes the tab bar

		m_ViewportFocused = ImGui::IsWindowFocused();
		m_ViewportHovered = ImGui::IsWindowHovered();

		Application::Get().GetImGuiLayer()->BlockEvents(!m_ViewportHovered);

		m_ViewportSize = { ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y };

		/// Render the viewport into an image
		const uint64_t textureID = m_ActiveScene->GetEditorFramebuffer()->GetColorAttachmentRendererID();
		ImGui::Image(textureID, ImVec2{ m_ViewportSize.x, m_ViewportSize.y }, ImVec2{ 0, 1 }, ImVec2{ 1, 0 });

		HandleDragAndDrop();

		/// Set the bounds of the viewport
		const auto windowSize = ImGui::GetWindowSize();
		ImVec2 minBound = ImGui::GetWindowPos();
		minBound.x += viewportOffset.x;
		minBound.y += viewportOffset.y;

		ImVec2 maxBound = { minBound.x + windowSize.x, minBound.y + windowSize.y };
		m_ViewportBounds[0] = { minBound.x, minBound.y };
		m_ViewportBounds[1] = { maxBound.x, maxBound.y };

		/// Gizmos
		const bool gizmosEnabled = m_SceneState == SceneState::Edit || m_SceneState == SceneState::Simulate;
		if (const Entity selectedEntity = m_HierarchyPanel.GetSelectedEntity(); selectedEntity && m_GizmoType != -1 && gizmosEnabled)
		{
			ImGuizmo::SetOrthographic(false);
			ImGuizmo::SetDrawlist();

			const float windowWidth = ImGui::GetWindowWidth();
			const float windowHeight = ImGui::GetWindowHeight();
			ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, windowWidth, windowHeight);

			/// Will be used for the Runtime camera
			/*const Entity cameraEntity = m_ActiveScene->GetPrimaryCameraEntity();
			const auto& camera = cameraEntity.GetComponent<CameraComponent>().Camera;

			const glm::mat4 cameraProjection = camera.GetProjection();
			const glm::mat4 cameraView = glm::inverse(cameraEntity.GetComponent<TransformComponent>().GetTransform());*/

			const glm::mat4 cameraProjection = m_EditorCamera.GetProjection();
			const glm::mat4 cameraView = m_EditorCamera.GetViewMatrix();

			/// Entity transform
			auto& tc = selectedEntity.GetComponent<TransformComponent>();
			auto transform = tc.GetTransform();

			/// Snapping 
			const bool snap = Input::IsKeyPressed(Key::LeftControl);
			float snapValue = 0.5f;
			if (m_GizmoType == ImGuizmo::OPERATION::ROTATE)
				snapValue = 45.0f;

			const float snapValues[3] = { snap ? snapValue : 0.0f, snap ? snapValue : 0.0f, snap ? snapValue : 0.0f };

			ImGuizmo::Manipulate(glm::value_ptr(cameraView), glm::value_ptr(cameraProjection),
				static_cast<ImGuizmo::OPERATION>(m_GizmoType), ImGuizmo::LOCAL, glm::value_ptr(transform), nullptr, snapValues);

			if (ImGuizmo::IsUsing())
			{
				glm::vec3 translation, rotationDegrees, scale;
				ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(transform), glm::value_ptr(translation), glm::value_ptr(rotationDegrees), glm::value_ptr(scale));

				tc.Translation = translation;
				tc.Rotation = glm::radians(rotationDegrees);
				tc.Scale = scale;

				CalculateEntityTransform(selectedEntity);
			}
		}

		ImGui::End();

		UIToolbar();

		m_NotificationManager.RenderNotifications();

		ImGui::End();

#ifdef KBR_DEBUG
		if (Input::IsKeyPressed(Key::RightControl))
		{
			ImGui::DebugStartItemPicker();
		}
#endif
	}

	void EditorLayer::OnEvent(Event& event)
	{
		m_CameraController.OnEvent(event);
		m_EditorCamera.OnEvent(event);

		EventDispatcher dispatcher(event);
		dispatcher.Dispatch<KeyPressedEvent>(KBR_BIND_EVENT_FN(EditorLayer::OnKeyPressed));
		dispatcher.Dispatch<MouseButtonPressedEvent>(KBR_BIND_EVENT_FN(EditorLayer::OnMouseButtonPressed));
		dispatcher.Dispatch<WindowDropEvent>(KBR_BIND_EVENT_FN(EditorLayer::OnWindowDrop));
	}

	bool EditorLayer::OnKeyPressed(const KeyPressedEvent& event)
	{
		/// Shortcuts
		if (event.GetRepeatCount() > 0)
			return false;

		const bool ctrl = Input::IsKeyPressed(Key::LeftControl) || Input::IsKeyPressed(Key::RightControl);
		const bool shift = Input::IsKeyPressed(Key::LeftShift) || Input::IsKeyPressed(Key::RightShift);

		switch (event.GetKeyCode())
		{
		case Key::S:
			if (ctrl && shift)
			{
				SaveSceneAs();
			}
			else if (ctrl)
			{
				SaveScene();
			}
			break;
		case Key::N:
			if (ctrl)
			{
				NewScene();
			}
			break;
		case Key::O:
			if (ctrl)
			{
				LoadScene();
			}
			break;

			/// Gizmos
		case Key::Q:
			m_GizmoType = -1;
			break;
		case Key::W:
			m_GizmoType = ImGuizmo::OPERATION::TRANSLATE;
			break;
		case Key::E:
			m_GizmoType = ImGuizmo::OPERATION::SCALE;
			break;
		case Key::R:
			m_GizmoType = ImGuizmo::OPERATION::ROTATE;
			break;
		default:
			break;
		}

		return false;
	}

	bool EditorLayer::OnMouseButtonPressed(const MouseButtonPressedEvent& event)
	{
		/// Handle mouse picking
		/// Only select the entity if we are not using the gizmos or the camera
		if (event.GetMouseButton() == Mouse::ButtonLeft && !ImGuizmo::IsOver() && !Input::IsKeyPressed(Key::LeftAlt))
		{
			m_HierarchyPanel.SetSelectedEntity(m_HoveredEntity);
		}
		return false;
	}

	bool EditorLayer::OnWindowDrop(const WindowDropEvent& event) 
	{
		/// TODO: Implement when asset manager is done
		return false;
	}

	void EditorLayer::OnScenePlay()
	{
		m_SceneState = SceneState::Play;

		m_ActiveScene->OnRuntimeStart();
	}

	void EditorLayer::OnSceneStop()
	{
		m_SceneState = SceneState::Edit;

		m_ActiveScene->OnRuntimeStop();
	}

	void EditorLayer::HandleDragAndDrop()
	{
		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_BROWSER_ITEM"))
			{
				const auto& path = static_cast<const char*>(payload->Data);
				KBR_INFO("Drag and drop payload: {0}", path);

				const std::string message = "Drag and drop payload: " + std::string(path);
				m_NotificationManager.AddNotification(message, Notification::Type::Info);

				/*const AssetHandle assetHandle = *static_cast<AssetHandle*>(payload->Data);
				const AssetType assetType = AssetManager::GetAssetType(assetHandle);
				if (assetType == AssetType::Texture2D)
				{
					const Ref<Texture2D> texture = AssetManager::GetAsset<Texture2D>(assetHandle);
					m_SquareColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
					m_Texture = texture;
				}*/
			}
			ImGui::EndDragDropTarget();
		}
	}

	void EditorLayer::CalculateEntityTransform(const Entity& entity) const
	{
		m_ActiveScene->CalculateEntityTransform(entity);
	}

	void EditorLayer::NewScene()
	{
		m_ActiveScene = CreateRef<Scene>();
		m_ActiveScene->OnViewportResize(static_cast<uint32_t>(m_ViewportSize.x), static_cast<uint32_t>(m_ViewportSize.y));
		m_HierarchyPanel.SetContext(m_ActiveScene);
	}

	void EditorLayer::UIToolbar()
	{
		constexpr ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;
		ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, ImVec2(0.0f, 0.0f));
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 2.0f));

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
		const auto& colors = ImGui::GetStyle().Colors;
		const auto& buttonHovered = colors[ImGuiCol_ButtonHovered];
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(buttonHovered.x, buttonHovered.y, buttonHovered.z, 0.5f));
		const auto& buttonActive = colors[ImGuiCol_ButtonActive];
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(buttonActive.x, buttonActive.y, buttonActive.z, 0.5f));

		ImGui::Begin("Toolbar", nullptr, flags);

		const Ref<Texture2D> icon = m_SceneState == SceneState::Edit ? m_IconPlay : m_IconStop;
		const float size = ImGui::GetWindowHeight() - 4.0f;

		ImGui::SetCursorPosX((ImGui::GetWindowContentRegionMax().x * 0.5f) - (size * 0.5f));

		if (ImGui::ImageButton("PlayButton", icon->GetRendererID(), ImVec2(size, size), ImVec2(0, 0), ImVec2(1, 1)))
		{
			if (m_SceneState == SceneState::Edit)
			{
				OnScenePlay();
			}
			else if (m_SceneState == SceneState::Play)
			{
				OnSceneStop();
			}
		}

		ImGui::PopStyleColor(3);
		ImGui::PopStyleVar(2);

		ImGui::End();
	}

	void EditorLayer::NewProject()
	{
		const auto newProject = Project::New();

		NewScene();
		const std::string newSceneName = "Unnamed Scene.kerberos";
		const std::filesystem::path scenePath = Project::GetAssetDirectory() / "scenes" / newSceneName;

		const SceneSerializer serializer(m_ActiveScene);
		serializer.Serialize(scenePath.string());

		m_NotificationManager.AddNotification("Scene saved to " + scenePath.string(), Notification::Type::Info);

		ProjectInfo projInfo;
		projInfo.Name = newProject->GetInfo().Name;
		projInfo.StartScenePath = "scenes/" + newSceneName;
		projInfo.AssetDirectory = Project::GetAssetDirectory();
		newProject->SetInfo(projInfo);

		m_AssetsPanel = CreateScope<AssetsPanel>(m_NotificationManager);
	}

	void EditorLayer::OpenProject(const std::filesystem::path& filepath)
	{
		if (const auto project = Project::Load(filepath))
		{
			const auto startScenePath = Project::GetAssetDirectory() / project->GetInfo().StartScenePath;
			OpenScene(startScenePath);

			m_AssetsPanel = CreateScope<AssetsPanel>(m_NotificationManager);
		}
	}

	bool EditorLayer::OpenProject()
	{
		const std::string filepathString = FileDialog::OpenFile("Kerberos Project (*.kbrproj)\0*.kbrproj\0");

		if (filepathString.empty())
			return false;

		OpenProject(filepathString);
		return true;
	}

	void EditorLayer::SaveScene()
	{
		const std::filesystem::path scenePath = "assets/scenes/Example.kerberos";

		const SceneSerializer serializer(m_ActiveScene);
		serializer.Serialize(scenePath.string());

		m_NotificationManager.AddNotification("Scene saved to " + scenePath.string(), Notification::Type::Info);

		Project::SaveActive();
	}

	void EditorLayer::SaveSceneAs()
	{
		const std::string filepath = FileDialog::SaveFile("Kerberos Scene (*.kerberos)\0*.kerberos\0");
		if (filepath.empty())
			return;

		const SceneSerializer serializer(m_ActiveScene);
		serializer.Serialize(filepath);

		m_NotificationManager.AddNotification("Scene saved to " + filepath, Notification::Type::Info);

		Project::SaveActive();
	}

	void EditorLayer::LoadScene()
	{
		const std::string filepathString = FileDialog::OpenFile("Kerberos Scene (*.kerberos)\0*.kerberos\0");

		if (filepathString.empty())
			return;

		/// Create a new scene, since otherwise the deserialized entities would be added to the current scene
		m_ActiveScene = CreateRef<Scene>();
		m_ActiveScene->OnViewportResize(static_cast<uint32_t>(m_ViewportSize.x), static_cast<uint32_t>(m_ViewportSize.y));
		m_HierarchyPanel.SetContext(m_ActiveScene);

		const SceneSerializer serializer(m_ActiveScene);
		if (!serializer.Deserialize(filepathString))
		{
			KBR_ERROR("Failed to load scene from {0}", filepathString);
		}
	}

	void EditorLayer::OpenScene(const std::filesystem::path& filepath)
	{
		m_ActiveScene = CreateRef<Scene>();
		m_ActiveScene->OnViewportResize(static_cast<uint32_t>(m_ViewportSize.x), static_cast<uint32_t>(m_ViewportSize.y));
		m_HierarchyPanel.SetContext(m_ActiveScene);

		const SceneSerializer serializer(m_ActiveScene);
		if (!serializer.Deserialize(filepath))
		{
			KBR_ERROR("Failed to load scene from {0}", filepath.string());
		}
	}
}
