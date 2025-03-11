#include "Sandbox2D.h"

#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/type_ptr.inl>
#include "imgui/imgui.h"

#define PROFILE_SCOPE(name) Kerberos::Timer timer##__LINE__(name, 

Sandbox2D::Sandbox2D()
	: Layer("Sandbox2D"), m_CameraController(1280.0f / 720.0f)
{
	m_Texture = Kerberos::Texture2D::Create("assets/textures/y2k_ice_texture.png");
}

void Sandbox2D::OnAttach()
{
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
