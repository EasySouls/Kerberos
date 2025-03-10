#include "Sandbox2D.h"

#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/type_ptr.inl>
#include "imgui/imgui.h"

#define PROFILE_SCOPE(name) Kerberos::Timer timer##__LINE__(name, [&](ProfileResult profileResult) { m_ProfileResults.push_back(profileResult); })

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

	{
		KBR_PROFILE_SCOPE("CameraController::OnUpdate");
		m_CameraController.OnUpdate(deltaTime);
	}

	Kerberos::RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1 });
	Kerberos::RenderCommand::Clear();

	{
		static float rotation = 0.0f;
		rotation += deltaTime * 20.0f;

		KBR_PROFILE_SCOPE("Renderer2D Draw");
		Kerberos::Renderer2D::BeginScene(m_CameraController.GetCamera());

		Kerberos::Renderer2D::DrawQuad({ -0.1f, 0.0f, 1.0f }, { 1.0f, 1.0f }, 10.0f, m_SquareColor);
		Kerberos::Renderer2D::DrawQuad({ 1.0f, 0.0f, 0.9f }, { 1.2f, 1.2f }, 0.0f, { 0.2f, 0.3f, 0.8f, 1.0f });
		Kerberos::Renderer2D::DrawQuad({ 0.0f, 0.0f, -0.99f }, { 5.0f, 5.0f }, rotation, m_Texture, 5);

		Kerberos::Renderer2D::EndScene();
	}
}

void Sandbox2D::OnImGuiRender()
{
	ImGui::Begin("Settings");
	ImGui::ColorEdit3("Square Color", glm::value_ptr(m_SquareColor));

	for (const auto& [Name, Time] : m_ProfileResults)
	{
		char label[50];
		strcpy_s(label, "%.3fms ");
		strcat_s(label, Name);
		ImGui::Text(label, Time);
	}
	m_ProfileResults.clear();

	ImGui::End();
}

void Sandbox2D::OnEvent(Kerberos::Event& event)
{
	m_CameraController.OnEvent(event);
}
