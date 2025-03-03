#include "Sandbox2D.h"

#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/type_ptr.inl>
#include "imgui/imgui.h"


Sandbox2D::Sandbox2D()
	: Layer("Sandbox2D"), m_CameraController(1280.0f / 720.0f)
{
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
	m_CameraController.OnUpdate(deltaTime);

	Kerberos::RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1 });
	Kerberos::RenderCommand::Clear();

	Kerberos::Renderer2D::BeginScene(m_CameraController.GetCamera());

	Kerberos::Renderer2D::DrawQuad({ -0.5f, 0.0f, 0.1f }, { 1.0f, 1.0f }, m_SquareColor);
	Kerberos::Renderer2D::DrawQuad({ 1.0f, 0.0f }, { 1.2f, 1.2f }, { 0.2f, 0.3f, 0.8f, 1.0f });

	Kerberos::Renderer2D::EndScene();
}

void Sandbox2D::OnImGuiRender()
{
	ImGui::Begin("Settigs");
	ImGui::ColorEdit3("Square Color", glm::value_ptr(m_SquareColor));
	ImGui::End();
}

void Sandbox2D::OnEvent(Kerberos::Event& event)
{
	m_CameraController.OnEvent(event);
}
