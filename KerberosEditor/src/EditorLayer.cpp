#include "EditorLayer.h"

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/type_ptr.inl>
#include "imgui/imgui.h"

#define PROFILE_SCOPE(name) Timer timer##__LINE__(name, 

static const char* s_Map =
"WWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW"
"WWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW"
"WWWWWWWWWWDDDDDDDDDDDDDWWWWWWWWW"
"WWWWWWWWDGGGGGGGGGGGGGGDWWWWWWWW"
"WWWWWWWDGGGGGGGGGGGGGGGGDWWWWWWW"
"WWWWWWWDGGGGGGGGGGGGGGGGDWWWWWWW"
"WWWWWWWDGGGGGGGGGGGGGGGGDWWWWWWW"
"WWWWWWWDGGGGGGGGGGGGGGGGDWWWWWWW"
"WWWWWWWWDGGGGGGGGGGGGGGDWWWWWWWW"
"WWWWWWWWWDDDDDDDDDDDDDDWWWWWWWWW"
"WWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW"
"WWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW"
"WWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW"
"WWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW"
"WWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW"
"WWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW";

namespace Kerberos
{
	EditorLayer::EditorLayer()
		: Layer("EditorLayer"), m_CameraController(1280.0f / 720.0f)
	{}

	void EditorLayer::OnAttach()
	{
		KBR_PROFILE_FUNCTION();

		FramebufferSpecification frameBuferSpec;
		frameBuferSpec.Width = 1280;
		frameBuferSpec.Height = 720;
		m_Framebuffer = Framebuffer::Create(frameBuferSpec);

		m_ActiveScene = CreateRef<Scene>();

		m_ViewportSize = { 1280.0f, 720.0f };

		m_Texture = Texture2D::Create("assets/textures/y2k_ice_texture.png");

		m_SpriteSheet = Texture2D::Create("assets/game/textures/RPGpack_sheet_2X.png");

		m_TextureStairs = SubTexture2D::CreateFromCoords(m_SpriteSheet, { 7, 6 }, { 128, 128 }, { 1, 1 });
		m_TextureBarrel = SubTexture2D::CreateFromCoords(m_SpriteSheet, { 8, 3 }, { 128, 128 }, { 1, 1 });
		m_TextureTree = SubTexture2D::CreateFromCoords(m_SpriteSheet, { 2, 1 }, { 128, 128 }, { 1, 2 });

		m_TextureGrass = SubTexture2D::CreateFromCoords(m_SpriteSheet, { 1, 11 }, { 128, 128 }, { 1, 1 });
		m_TextureDirt = SubTexture2D::CreateFromCoords(m_SpriteSheet, { 6, 11 }, { 128, 128 }, { 1, 1 });
		m_TextureWater = SubTexture2D::CreateFromCoords(m_SpriteSheet, { 11, 11 }, { 128, 128 }, { 1, 1 });

		m_TileMap['G'] = m_TextureGrass;
		m_TileMap['D'] = m_TextureDirt;
		m_TileMap['W'] = m_TextureWater;

		m_Particle = ParticleProps{
			.Position = { 0.0f, 0.0f },
			.Velocity = { 0.0f, 0.0f },
			.VelocityVariation = { 3.0f, 1.0f },
			.ColorBegin = { 0.8f, 0.3f, 0.2f, 1.0f },
			.ColorEnd = { 0.2f, 0.3f, 0.8f, 1.0f },
			.SizeBegin = 0.2f,
			.SizeEnd = 0.0f,
			.SizeVariation = 0.3f,
			.LifeTime = 1.0f
		};

		m_CameraController.SetZoomLevel(5.0f);

		Entity squareEntity = m_ActiveScene->CreateEntity("Square");
		squareEntity.AddComponent<SpriteRendererComponent>(glm::vec4{ 0.2f, 0.3f, 0.8f, 1.0f });

		m_CameraEntity = m_ActiveScene->CreateEntity("Camera");
		m_CameraEntity.AddComponent<CameraComponent>();

		m_SecondCamera = m_ActiveScene->CreateEntity("Second Camera");
		auto& secondCameraComponent = m_SecondCamera.AddComponent<CameraComponent>();
		secondCameraComponent.IsPrimary = false;
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

		{
			KBR_PROFILE_SCOPE("CameraController::OnUpdate");
			m_CameraController.OnUpdate(deltaTime);
		}

		Renderer2D::ResetStatistics();
		{
			KBR_PROFILE_SCOPE("Renderer Prep");

			m_Framebuffer->Bind();

			RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1 });
			RenderCommand::Clear();
		}

#if 0
		{
			static float rotation = 0.0f;
			rotation += deltaTime * 20.0f;

			KBR_PROFILE_SCOPE("Renderer2D Draw");
			Renderer2D::BeginScene(m_CameraController.GetCamera());

			//Renderer2D::DrawTexturedQuad({ -0.1f, 0.0f, 1.0f }, { 1.0f, 1.0f }, 10.0f, m_SquareColor);
			Renderer2D::DrawTexturedQuad({ 0.0f, 0.0f, -0.9f }, { 5.0f, 5.0f }, rotation, m_Texture, 5);
			Renderer2D::DrawTexturedQuad({ 0.0f, 0.0f, -0.9f }, { 10.0f, 10.0f }, rotation, m_Texture, 1);
			Renderer2D::DrawTexturedQuad({ 1.0f, 0.0f, 0.0f }, { 1.2f, 1.2f }, 0.0f, { 0.2f, 0.3f, 0.8f, 1.0f });

			for (float y = -5.0f; y < 5.0f; y += 0.5f)
			{
				for (float x = -5.0f; x < 5.0f; x += 0.5f)
				{
					glm::vec4 color = { (x + 5.0f) / 10.0f, 0.4f, (y + 5.0f) / 10.0f, 0.5f };
					Renderer2D::DrawTexturedQuad({ x, y, 0.0f }, { 0.45f, 0.45f }, 0.0f, color);
				}
			}

			Renderer2D::EndScene();
		}
#endif

		{
			//Renderer2D::BeginScene(m_CameraController.GetCamera());

#if TEXTURE_EXAMPLE
			Renderer2D::DrawTexturedQuad({ 0.0f, 0.0f, 0.4f }, { 1.0f, 1.0f }, 0.0f, m_TextureStairs);
			Renderer2D::DrawTexturedQuad({ 1.0f, 0.0f, 0.4f }, { 1.0f, 1.0f }, 0.0f, m_TextureBarrel);
			Renderer2D::DrawTexturedQuad({ -1.0f, 0.0f, 0.4f }, { 1.0f, 2.0f }, 0.0f, m_TextureTree);
#endif


#if MAP_EXAMPLE
			for (size_t y = 0; y < m_MapHeight; y++)
			{
				for (size_t x = 0; x < m_MapWidth; x++)
				{
					char tileType = s_Map[x + y * m_MapWidth];
					Ref<SubTexture2D> texture;

					if (m_TileMap.contains(tileType))
					{
						texture = m_TileMap[tileType];
					}

					else
						texture = m_TextureGrass;
					Renderer2D::DrawTexturedQuad({ x - m_MapWidth / 2.0f, m_MapHeight - y - m_MapHeight / 2.0f, 0.0f }, { 1.0f, 1.0f }, 0.0f, texture);
				}
			}
#endif

			m_ActiveScene->OnUpdate(deltaTime);

			//Renderer2D::EndScene();

			m_Framebuffer->Unbind();
		}

		if (Input::IsMouseButtonPressed(KBR_MOUSE_BUTTON_LEFT))
		{
			auto [x, y] = Input::GetMousePosition();

			const auto width = Application::Get().GetWindow().GetWidth();
			const auto height = Application::Get().GetWindow().GetHeight();
			const auto& bounds = m_CameraController.GetBounds();

			const auto& pos = m_CameraController.GetCamera().GetPosition();

			const float halfWidth = static_cast<float>(width) * 0.5f;
			const float halfHeight = static_cast<float>(height) * 0.5f;

			x = ((x - halfWidth) / static_cast<float>(width)) * bounds.GetWidth();
			y = ((halfHeight - y) / static_cast<float>(height)) * bounds.GetHeight();

			m_Particle.Position = { x + pos.x, y + pos.y };

			for (int i = 0; i < 10; i++)
				m_ParticleSystem.Emit(m_Particle);
		}

		m_ParticleSystem.OnUpdate(deltaTime);
		m_ParticleSystem.OnRender(m_CameraController.GetCamera());
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
				if (ImGui::MenuItem("Exit")) Application::Get().Close();
				ImGui::EndMenu();
			}
			ImGui::EndMenuBar();
		}


		ImGui::Begin("Settings");
		ImGui::ColorEdit3("Square Color", glm::value_ptr(m_SquareColor));

		const auto stats = Renderer2D::GetStatistics();
		ImGui::Text("Renderer2D Stats");
		ImGui::Text("Draw Calls: %u", stats.DrawCalls);
		ImGui::Text("Quads: %u", stats.QuadCount);
		ImGui::Text("Vertices: %u", stats.GetTotalVertexCount());
		ImGui::Text("Indices: %u", stats.GetTotalIndexCount());

		for (const auto& [Name, Time] : m_ProfileResults)
		{
			const auto fmt = "%s %.3fms";
			ImGui::Text(fmt, Name, Time);
		}
		m_ProfileResults.clear();

		ImGui::Text("FPS: %.2f", m_Fps);

		ImGui::Text("Camera Stats");
		const auto& camera = m_CameraController.GetCamera();
		ImGui::Text("Position: %.1f, %.1f, %.1f", camera.GetPosition().x, camera.GetPosition().y, camera.GetPosition().z);
		ImGui::Text("Rotation: %.1f", camera.GetRotation());
		ImGui::Text("Zoom Level: %.1f", m_CameraController.GetZoomLevel());

		ImGui::End();

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::Begin("Viewport");
		ImGui::PopStyleVar();

		// TODO: The resizing of the viewport can be moved to OnUpdate

		const glm::vec2 viewportPanelSize = { ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y };
		if (m_ViewportSize != viewportPanelSize && viewportPanelSize.x > 0 && viewportPanelSize.y > 0)
		{
			m_ViewportSize = viewportPanelSize;
			m_Framebuffer->Resize(static_cast<uint32_t>(viewportPanelSize.x), static_cast<uint32_t>(viewportPanelSize.y));
			m_CameraController.OnResize(viewportPanelSize.x, viewportPanelSize.y);

			m_ActiveScene->OnViewportResize(static_cast<uint32_t>(viewportPanelSize.x), static_cast<uint32_t>(viewportPanelSize.y));
		}

		const uint32_t textureID = m_Framebuffer->GetColorAttachmentRendererID();
		ImGui::Image(textureID, ImVec2{ m_ViewportSize.x, m_ViewportSize.y }, ImVec2{ 0, 1 }, ImVec2{ 1, 0 });
		ImGui::End();

		ImGui::End();
	}

	void EditorLayer::OnEvent(Event& event)
	{
		m_CameraController.OnEvent(event);
	}
}