#include "Sandbox2D.h"

#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/type_ptr.inl>
#include "imgui/imgui.h"

#define PROFILE_SCOPE(name) Kerberos::Timer timer##__LINE__(name, 

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


Sandbox2D::Sandbox2D()
	: Layer("Sandbox2D"), m_CameraController(1280.0f / 720.0f)
{
}

void Sandbox2D::OnAttach()
{
	KBR_PROFILE_FUNCTION();

	m_Texture = Kerberos::Texture2D::Create("assets/textures/y2k_ice_texture.png");

	m_SpriteSheet = Kerberos::Texture2D::Create("assets/game/textures/RPGpack_sheet_2X.png");

	m_TextureStairs = Kerberos::SubTexture2D::CreateFromCoords(m_SpriteSheet, { 7, 6 }, { 128, 128 }, { 1, 1 });
	m_TextureBarrel = Kerberos::SubTexture2D::CreateFromCoords(m_SpriteSheet, { 8, 3 }, { 128, 128 }, { 1, 1 });
	m_TextureTree = Kerberos::SubTexture2D::CreateFromCoords(m_SpriteSheet, { 2, 1 }, { 128, 128 }, { 1, 2 });

	m_TextureGrass = Kerberos::SubTexture2D::CreateFromCoords(m_SpriteSheet, { 1, 11 }, { 128, 128 }, { 1, 1 });
	m_TextureDirt = Kerberos::SubTexture2D::CreateFromCoords(m_SpriteSheet, { 6, 11 }, { 128, 128 }, { 1, 1 });
	m_TextureWater = Kerberos::SubTexture2D::CreateFromCoords(m_SpriteSheet, { 11, 11 }, { 128, 128 }, { 1, 1 });

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
}

void Sandbox2D::OnDetach()
{
	Layer::OnDetach();
}

void Sandbox2D::OnUpdate(const Kerberos::Timestep deltaTime)
{
	KBR_PROFILE_FUNCTION();

	m_Fps = static_cast<float>(1) / deltaTime;

	Kerberos::Timer timer("Sandbox2D::OnUpdate", [&](const ProfileResult profileResult) { m_ProfileResults.push_back(profileResult); });

	{
		KBR_PROFILE_SCOPE("CameraController::OnUpdate");
		m_CameraController.OnUpdate(deltaTime);
	}

	Kerberos::Renderer2D::ResetStatistics();

	Kerberos::RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1 });
	Kerberos::RenderCommand::Clear();

#if 0
	{
		static float rotation = 0.0f;
		rotation += deltaTime * 20.0f;

		KBR_PROFILE_SCOPE("Renderer2D Draw");
		Kerberos::Renderer2D::BeginScene(m_CameraController.GetCamera());

		//Kerberos::Renderer2D::DrawQuad({ -0.1f, 0.0f, 1.0f }, { 1.0f, 1.0f }, 10.0f, m_SquareColor);
		Kerberos::Renderer2D::DrawQuad({ 0.0f, 0.0f, -0.9f }, { 5.0f, 5.0f }, rotation, m_Texture, 5);
		Kerberos::Renderer2D::DrawQuad({ 0.0f, 0.0f, -0.9f }, { 10.0f, 10.0f }, rotation, m_Texture, 1);
		Kerberos::Renderer2D::DrawQuad({ 1.0f, 0.0f, 0.0f }, { 1.2f, 1.2f }, 0.0f, { 0.2f, 0.3f, 0.8f, 1.0f });

		for (float y = -5.0f; y < 5.0f; y += 0.5f)
		{
			for (float x = -5.0f; x < 5.0f; x += 0.5f)
			{
				glm::vec4 color = { (x + 5.0f) / 10.0f, 0.4f, (y + 5.0f) / 10.0f, 0.5f };
				Kerberos::Renderer2D::DrawQuad({ x, y, 0.0f }, { 0.45f, 0.45f }, 0.0f, color);
			}
		}

		Kerberos::Renderer2D::EndScene();
	}
#endif

	{
		Kerberos::Renderer2D::BeginScene(m_CameraController.GetCamera());

		/*Kerberos::Renderer2D::DrawTexturedQuad({ 0.0f, 0.0f, 0.4f }, { 1.0f, 1.0f }, 0.0f, m_TextureStairs);
		Kerberos::Renderer2D::DrawTexturedQuad({ 1.0f, 0.0f, 0.4f }, { 1.0f, 1.0f }, 0.0f, m_TextureBarrel);
		Kerberos::Renderer2D::DrawTexturedQuad({ -1.0f, 0.0f, 0.4f }, { 1.0f, 2.0f }, 0.0f, m_TextureTree);*/

		for (size_t y = 0; y < m_MapHeight; y++)
		{
			for (size_t x = 0; x < m_MapWidth; x++)
			{
				char tileType = s_Map[x + y * m_MapWidth];
				Kerberos::Ref<Kerberos::SubTexture2D> texture;

				if (m_TileMap.contains(tileType))
				{
					texture = m_TileMap[tileType];
				}

				else
					texture = m_TextureGrass;
				Kerberos::Renderer2D::DrawTexturedQuad({ x - m_MapWidth / 2.0f, m_MapHeight - y - m_MapHeight / 2.0f, 0.0f }, { 1.0f, 1.0f }, 0.0f, texture);
			}
		}

		Kerberos::Renderer2D::EndScene();
	}

	if (Kerberos::Input::IsMouseButtonPressed(KBR_MOUSE_BUTTON_LEFT))
	{
		auto [x, y] = Kerberos::Input::GetMousePosition();

		const auto width = Kerberos::Application::Get().GetWindow().GetWidth();
		const auto height = Kerberos::Application::Get().GetWindow().GetHeight();
		const auto bounds = m_CameraController.GetBounds();

		const auto pos = m_CameraController.GetCamera().GetPosition();

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

void Sandbox2D::OnImGuiRender()
{
	ImGui::Begin("Settings");
	ImGui::ColorEdit3("Square Color", glm::value_ptr(m_SquareColor));

	const auto stats = Kerberos::Renderer2D::GetStatistics();
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

	ImGui::End();
}

void Sandbox2D::OnEvent(Kerberos::Event& event)
{
	m_CameraController.OnEvent(event);
}
