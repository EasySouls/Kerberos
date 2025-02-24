#include <Kerberos.h>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/type_ptr.inl>

#include "imgui/imgui.h"
#include "Platform/OpenGL/OpenGLShader.h"

class ExampleLayer : public Kerberos::Layer
{
public:
	ExampleLayer()
		: Layer("Example"), m_Camera(-1.6f, 1.6f, -0.9f, 0.9f), m_CameraPosition(0.0f), m_SquarePosition(0.0f)
	{
		m_VertexArray.reset(Kerberos::VertexArray::Create());

		constexpr float vertices[3 * 7] = {
			-0.5f, -0.5f, 0.0f, 0.9f, 0.3f, 1.0f, 1.0f,
			 0.5f, -0.5f, 0.0f, 0.9f, 0.3f, 1.0f, 1.0f,
			 0.0f,  0.5f, 0.0f, 0.9f, 0.3f, 1.0f, 1.0f,
		};

		Kerberos::Ref<Kerberos::VertexBuffer> vertexBuffer;
		vertexBuffer.reset(Kerberos::VertexBuffer::Create(vertices, sizeof(vertices)));

		Kerberos::BufferLayout layout = {
			{ Kerberos::ShaderDataType::Float3, "a_Position" },
			{ Kerberos::ShaderDataType::Float4, "a_Color" }
		};

		vertexBuffer->SetLayout(layout);
		m_VertexArray->AddVertexBuffer(vertexBuffer);

		constexpr uint32_t indices[3] = { 0, 1, 2 };

		Kerberos::Ref<Kerberos::IndexBuffer> indexBuffer;
		indexBuffer.reset(Kerberos::IndexBuffer::Create(indices, sizeof(indices) / sizeof(uint32_t)));
		m_VertexArray->SetIndexBuffer(indexBuffer);

		m_SquareVA.reset(Kerberos::VertexArray::Create());

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

		m_SquareVA->AddVertexBuffer(squareVB);

		constexpr uint32_t squareIndices[6] = { 0, 1, 2, 2, 3, 0 };

		Kerberos::Ref<Kerberos::IndexBuffer> squareIB;
		squareIB.reset(Kerberos::IndexBuffer::Create(squareIndices, sizeof(squareIndices) / sizeof(uint32_t)));

		m_SquareVA->SetIndexBuffer(squareIB);

		//m_FlatColorShader = Kerberos::Shader::Create("assets/shaders/flatcolor.glsl");

		auto textureShader = m_ShaderLib.Load("assets/shaders/texture.glsl");

		m_Texture = Kerberos::Texture2D::Create("assets/textures/y2k_ice_texture.png");
		std::dynamic_pointer_cast<Kerberos::OpenGLShader>(textureShader)->Bind();
		std::dynamic_pointer_cast<Kerberos::OpenGLShader>(textureShader)->UploadUniformInt("u_Texture", 0);
	}

	void OnUpdate(const Kerberos::Timestep deltaTime) override
	{
		m_Fps = static_cast<float>(1) / deltaTime;
		//KBR_TRACE("Delta time: {0}s ({1}ms)", deltaTime.GetSeconds(), deltaTime.GetMilliseconds());

		/// Move the camera in the x axis
		if (Kerberos::Input::IsKeyPressed(KBR_KEY_A))
			m_CameraPosition.x -= m_CameraMoveSpeed * deltaTime;
		else if (Kerberos::Input::IsKeyPressed(KBR_KEY_D))
			m_CameraPosition.x += m_CameraMoveSpeed * deltaTime;

		/// Move the camera in the y axis
		if (Kerberos::Input::IsKeyPressed(KBR_KEY_W))
			m_CameraPosition.y += m_CameraMoveSpeed * deltaTime;
		else if (Kerberos::Input::IsKeyPressed(KBR_KEY_S))
			m_CameraPosition.y -= m_CameraMoveSpeed * deltaTime;

		/// Rotate the camera
		if (Kerberos::Input::IsKeyPressed(KBR_KEY_Q))
			m_CameraRotation += m_CameraRotationSpeed * deltaTime;
		else if (Kerberos::Input::IsKeyPressed(KBR_KEY_E))
			m_CameraRotation -= m_CameraRotationSpeed * deltaTime;

		/// Move the square in the x axis
		if (Kerberos::Input::IsKeyPressed(KBR_KEY_H))
			m_SquarePosition.x -= m_SquareMoveSpeed * deltaTime;
		else if (Kerberos::Input::IsKeyPressed(KBR_KEY_K))
			m_SquarePosition.x += m_SquareMoveSpeed * deltaTime;

		/// Move the square in the y axis
		if (Kerberos::Input::IsKeyPressed(KBR_KEY_U))
			m_SquarePosition.y += m_SquareMoveSpeed * deltaTime;
		else if (Kerberos::Input::IsKeyPressed(KBR_KEY_J))
			m_SquarePosition.y -= m_SquareMoveSpeed * deltaTime;

		Kerberos::RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1 });
		Kerberos::RenderCommand::Clear();

		m_Camera.SetPosition(m_CameraPosition);
		m_Camera.SetRotation(m_CameraRotation);

		Kerberos::Renderer::BeginScene(m_Camera);

		//const glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(0.1f));

		//std::dynamic_pointer_cast<Kerberos::OpenGLShader>(m_FlatColorShader)->Bind();
		//std::dynamic_pointer_cast<Kerberos::OpenGLShader>(m_FlatColorShader)->UploadUniformFloat4("u_Color", m_SquareColor);

		//for (size_t y = 0; y < 20; y++)
		//{
		//	for (size_t x = 0; x < 20; x++)
		//	{
		//		glm::vec3 pos(x * 0.11f, y * 0.11f, 0.0f);
		//		glm::mat4 transform = glm::translate(glm::mat4(1.0f), pos) * scale;
		//		Kerberos::Renderer::Submit(m_FlatColorShader, m_SquareVA, transform);
		//	}
		//}

		auto textureShader = m_ShaderLib.Get("texture");
		m_Texture->Bind();
		Kerberos::Renderer::Submit(textureShader, m_SquareVA, glm::scale(glm::mat4(1.0f), glm::vec3(1.5f)));

		// Triangle
		//Kerberos::Renderer::Submit(m_Shader, m_VertexArray);

		Kerberos::Renderer::EndScene();
	}

	void OnImGuiRender() override
	{
		ImGui::Begin("Frame Data");
		ImGui::Text("FPS: %f", m_Fps);
		ImGui::Text("Camera Position: x=%f, y=%f, z=%f", m_CameraPosition.x, m_CameraPosition.y, m_CameraPosition.z);
		ImGui::Text("Camera Rotation: %f", m_CameraRotation);
		ImGui::Text("Square Position: x=%f, y=%f, z=%f", m_SquarePosition.x, m_SquarePosition.y, m_SquarePosition.z);
		ImGui::End();

		ImGui::Begin("Settings");
		ImGui::ColorEdit4("Square Color", glm::value_ptr(m_SquareColor));
		ImGui::End();
	}

	void OnEvent(Kerberos::Event& event) override
	{
		Kerberos::EventDispatcher dispatcher(event);
		dispatcher.Dispatch<Kerberos::KeyPressedEvent>(KBR_BIND_FN(OnKeyPressedEvent));
	}

	static bool OnKeyPressedEvent(const Kerberos::KeyPressedEvent& event)
	{
		KBR_CORE_TRACE("{0}", static_cast<char>(event.GetKeyCode()));

		return false;
	}

private:
	Kerberos::ShaderLibrary m_ShaderLib;
	Kerberos::Ref<Kerberos::VertexArray> m_VertexArray;

	Kerberos::Ref<Kerberos::Shader> m_FlatColorShader;
	Kerberos::Ref<Kerberos::VertexArray> m_SquareVA;

	Kerberos::Ref<Kerberos::Texture2D> m_Texture;

	glm::vec4 m_SquareColor = { 0.2f, 0.3f, 0.8f, 1.0f };

	Kerberos::OrthographicCamera m_Camera;
	glm::vec3 m_CameraPosition;
	float m_CameraRotation = 0.0f;

	float m_CameraMoveSpeed = 1.0f;
	float m_CameraRotationSpeed = 45.0f;

	glm::vec3 m_SquarePosition;
	float m_SquareMoveSpeed = 3.0f;

	float m_Fps = 0;
};

class Sandbox : public Kerberos::Application
{
public:
	Sandbox()
	{
		PushLayer(new ExampleLayer());
	}

	~Sandbox() override = default;
};

Kerberos::Application* Kerberos::CreateApplication()
{
	return new Sandbox();
}