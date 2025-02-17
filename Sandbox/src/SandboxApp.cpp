#include <Kerberos.h>
#include <glm/ext/matrix_transform.hpp>

#include "imgui/imgui.h"

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

		std::shared_ptr<Kerberos::VertexBuffer> vertexBuffer;
		vertexBuffer.reset(Kerberos::VertexBuffer::Create(vertices, sizeof(vertices)));

		Kerberos::BufferLayout layout = {
			{ Kerberos::ShaderDataType::Float3, "a_Position" },
			{ Kerberos::ShaderDataType::Float4, "a_Color" }
		};

		vertexBuffer->SetLayout(layout);
		m_VertexArray->AddVertexBuffer(vertexBuffer);

		constexpr uint32_t indices[3] = { 0, 1, 2 };

		std::shared_ptr<Kerberos::IndexBuffer> indexBuffer;
		indexBuffer.reset(Kerberos::IndexBuffer::Create(indices, sizeof(indices) / sizeof(uint32_t)));
		m_VertexArray->SetIndexBuffer(indexBuffer);

		m_SquareVA.reset(Kerberos::VertexArray::Create());

		constexpr float squareVertices[3 * 4] = {
			   -0.75f, -0.75f, 0.0f,
				0.75f, -0.75f, 0.0f,
				0.75f,  0.75f, 0.0f,
			   -0.75f,  0.75f, 0.0f
		};

		std::shared_ptr<Kerberos::VertexBuffer> squareVB;
		squareVB.reset(Kerberos::VertexBuffer::Create(squareVertices, sizeof(squareVertices)));

		squareVB->SetLayout({
			{ Kerberos::ShaderDataType::Float3, "a_Position" }
			});

		m_SquareVA->AddVertexBuffer(squareVB);

		constexpr uint32_t squareIndices[6] = { 0, 1, 2, 2, 3, 0 };

		std::shared_ptr<Kerberos::IndexBuffer> squareIB;
		squareIB.reset(Kerberos::IndexBuffer::Create(squareIndices, sizeof(squareIndices) / sizeof(uint32_t)));

		m_SquareVA->SetIndexBuffer(squareIB);

		const std::string vertexSrc = R"(
			#version 330 core
			
			layout(location=0) in vec3 a_Pos;
			layout(location=1) in vec4 a_Color;

			uniform mat4 u_ViewProjection;
			uniform mat4 u_Transform;

			out vec3 v_Pos;
			out vec4 v_Color;

			void main() {
				v_Pos = a_Pos;
				v_Color = a_Color;
				gl_Position = u_ViewProjection * u_Transform * vec4(a_Pos, 1.0);
			}
		)";

		const std::string fragmentSrc = R"(
			#version 330 core
			
			layout(location=0) out vec4 color;

			in vec3 v_Pos;
			in vec4 v_Color;

			void main() {
				color = v_Color;
			}
		)";

		m_Shader.reset(new Kerberos::Shader(vertexSrc, fragmentSrc));

		const std::string blueShaderVertexSrc = R"(
			#version 330 core
			
			layout(location = 0) in vec3 a_Position;

			uniform mat4 u_ViewProjection;
			uniform mat4 u_Transform;

			out vec3 v_Position;

			void main()
			{
				v_Position = a_Position;
				gl_Position = u_ViewProjection * u_Transform * vec4(a_Position, 1.0);	
			}
		)";

		const std::string blueShaderFragmentSrc = R"(
			#version 330 core
			
			layout(location = 0) out vec4 color;
			in vec3 v_Position;
			void main()
			{
				color = vec4(0.2, 0.3, 0.8, 1.0);
			}
		)";
		m_BlueShader.reset(new Kerberos::Shader(blueShaderVertexSrc, blueShaderFragmentSrc));
	}

	void OnUpdate(const Kerberos::Timestep deltaTime) override
	{
		KBR_TRACE("Delta time: {0}s ({1}ms)", deltaTime.GetSeconds(), deltaTime.GetMilliseconds());

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

		const glm::mat4 transform = glm::translate({ 1.0f }, m_SquarePosition);

		Kerberos::Renderer::Submit(m_BlueShader, m_SquareVA, transform);
		Kerberos::Renderer::Submit(m_Shader, m_VertexArray);

		Kerberos::Renderer::EndScene();
	}

	void OnImGuiRender() override
	{
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
	std::shared_ptr<Kerberos::Shader> m_Shader;
	std::shared_ptr<Kerberos::VertexArray> m_VertexArray;
	std::shared_ptr<Kerberos::Shader> m_BlueShader;
	std::shared_ptr<Kerberos::VertexArray> m_SquareVA;

	Kerberos::OrthographicCamera m_Camera;
	glm::vec3 m_CameraPosition;
	float m_CameraRotation = 0.0f;

	float m_CameraMoveSpeed = 1.0f;
	float m_CameraRotationSpeed = 45.0f;

	glm::vec3 m_SquarePosition;
	float m_SquareMoveSpeed = 3.0f;
};

class Sandbox : public Kerberos::Application
{
public:
	Sandbox()
	{
		PushLayer(new ExampleLayer());
	}

	~Sandbox() = default;
};

Kerberos::Application* Kerberos::CreateApplication()
{
	return new Sandbox();
}