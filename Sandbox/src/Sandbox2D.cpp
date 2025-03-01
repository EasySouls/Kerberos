#include "Sandbox2D.h"

#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/type_ptr.inl>
#include "imgui/imgui.h"
#include "Platform/OpenGL/OpenGLShader.h"


Sandbox2D::Sandbox2D()
	: Layer("Sandbox2D"), m_CameraController(1280.0f / 720.0f)
{
}

void Sandbox2D::OnAttach()
{
	m_VertexArray = Kerberos::VertexArray::Create();

	/// 3 positions, 2 texture coordinates
	constexpr float squareVertices[5 * 4] = {
		   -0.5f, -0.5f, 0.0f, 0.0f, 0.0f,
			0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
			0.5f,  0.5f, 0.0f, 1.0f, 1.0f,
		   -0.5f,  0.5f, 0.0f, 0.0f, 1.0f
	};

	Kerberos::Ref<Kerberos::VertexBuffer> squareVB;
	squareVB.reset(Kerberos::VertexBuffer::Create(squareVertices, sizeof(squareVertices)));

	squareVB->SetLayout({
		{ Kerberos::ShaderDataType::Float3, "a_Position" },
		{ Kerberos::ShaderDataType::Float2, "a_TexCoord" }
		});

	m_VertexArray->AddVertexBuffer(squareVB);

	constexpr uint32_t squareIndices[6] = { 0, 1, 2, 2, 3, 0 };

	Kerberos::Ref<Kerberos::IndexBuffer> squareIB;
	squareIB.reset(Kerberos::IndexBuffer::Create(squareIndices, sizeof(squareIndices) / sizeof(uint32_t)));

	m_VertexArray->SetIndexBuffer(squareIB);

	m_Shader = Kerberos::Shader::Create("assets/shaders/texture.glsl");

	m_Texture = Kerberos::Texture2D::Create("assets/textures/y2k_ice_texture.png");
	std::dynamic_pointer_cast<Kerberos::OpenGLShader>(m_Shader)->Bind();
	std::dynamic_pointer_cast<Kerberos::OpenGLShader>(m_Shader)->UploadUniformInt("u_Texture", 0);
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

	Kerberos::Renderer::BeginScene(m_CameraController.GetCamera());

	m_Texture->Bind();
	Kerberos::Renderer::Submit(m_Shader, m_VertexArray, glm::scale(glm::mat4(1.0f), glm::vec3(1.5f)));

	Kerberos::Renderer::EndScene();
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
