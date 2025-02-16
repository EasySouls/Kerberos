#include "kbrpch.h"
#include "Application.h"

#include "KeyCodes.h"
#include "Events/KeyEvent.h"
#include "Kerberos/Core.h"
#include "Kerberos/Renderer/Renderer.h"
#include "Kerberos/Renderer/RenderCommand.h"

namespace Kerberos
{
#define BIND_EVENT_FN(x) std::bind(&x, this, std::placeholders::_1)

	Application* Application::s_Instance = nullptr;

	Application::Application()
		: m_Camera(-1.6f, 1.6f, -0.9f, 0.9f)
	{
		KBR_CORE_ASSERT(!s_Instance, "Application already exists!");
		s_Instance = this;

		m_Window = std::unique_ptr<Window>(Window::Create());
		m_Window->SetEventCallback(BIND_EVENT_FN(Application::OnEvent));

		m_ImGuiLayer = new ImGuiLayer();
		PushOverlay(m_ImGuiLayer);

		m_VertexArray.reset(VertexArray::Create());

		constexpr float vertices[3 * 7] = {
			-0.5f, -0.5f, 0.0f, 0.9f, 0.3f, 1.0f, 1.0f,
			 0.5f, -0.5f, 0.0f, 0.9f, 0.3f, 1.0f, 1.0f,
			 0.0f,  0.5f, 0.0f, 0.9f, 0.3f, 1.0f, 1.0f,
		};

		std::shared_ptr<VertexBuffer> vertexBuffer;
		vertexBuffer.reset(VertexBuffer::Create(vertices, sizeof(vertices)));

		BufferLayout layout = {
			{ ShaderDataType::Float3, "a_Position" },
			{ ShaderDataType::Float4, "a_Color" }
		};

		vertexBuffer->SetLayout(layout);
		m_VertexArray->AddVertexBuffer(vertexBuffer);

		constexpr uint32_t indices[3] = { 0, 1, 2 };

		std::shared_ptr<IndexBuffer> indexBuffer;
		indexBuffer.reset(IndexBuffer::Create(indices, sizeof(indices) / sizeof(uint32_t)));
		m_VertexArray->SetIndexBuffer(indexBuffer);

		m_SquareVA.reset(VertexArray::Create());

		constexpr float squareVertices[3 * 4] = {
			   -0.75f, -0.75f, 0.0f,
				0.75f, -0.75f, 0.0f,
				0.75f,  0.75f, 0.0f,
			   -0.75f,  0.75f, 0.0f
		};

		std::shared_ptr<VertexBuffer> squareVB;
		squareVB.reset(VertexBuffer::Create(squareVertices, sizeof(squareVertices)));

		squareVB->SetLayout({
			{ ShaderDataType::Float3, "a_Position" }
		});

		m_SquareVA->AddVertexBuffer(squareVB);

		constexpr uint32_t squareIndices[6] = { 0, 1, 2, 2, 3, 0 };

		std::shared_ptr<IndexBuffer> squareIB;
		squareIB.reset(IndexBuffer::Create(squareIndices, sizeof(squareIndices) / sizeof(uint32_t)));

		m_SquareVA->SetIndexBuffer(squareIB);

		const std::string vertexSrc = R"(
			#version 330 core
			
			layout(location=0) in vec3 a_Pos;
			layout(location=1) in vec4 a_Color;

			uniform mat4 u_ViewProjection;

			out vec3 v_Pos;
			out vec4 v_Color;

			void main() {
				v_Pos = a_Pos;
				v_Color = a_Color;
				gl_Position = u_ViewProjection * vec4(a_Pos, 1.0);
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

		m_Shader.reset(new Shader(vertexSrc, fragmentSrc));

		const std::string blueShaderVertexSrc = R"(
			#version 330 core
			
			layout(location = 0) in vec3 a_Position;

			uniform mat4 u_ViewProjection;

			out vec3 v_Position;

			void main()
			{
				v_Position = a_Position;
				gl_Position = u_ViewProjection * vec4(a_Position, 1.0);	
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
		m_BlueShader.reset(new Shader(blueShaderVertexSrc, blueShaderFragmentSrc));
	}

	Application::~Application() = default;

	void Application::Run()
	{
		while (m_Running)
		{
			RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1 });
			RenderCommand::Clear();

			//m_Camera.SetRotation(15.0f);
			//m_Camera.SetPosition({ 0.25f, 0.25f, 0 });

			Renderer::BeginScene(m_Camera);

			Renderer::Submit(m_BlueShader, m_SquareVA);
			Renderer::Submit(m_Shader, m_VertexArray);

			Renderer::EndScene();

			for (Layer* layer : m_LayerStack)
				layer->OnUpdate();

			// TODO: Execute this on the render thread
			m_ImGuiLayer->Begin();
			for (Layer* layer : m_LayerStack)
				layer->OnImGuiRender();
			m_ImGuiLayer->End();

			m_Window->OnUpdate();
		}
	}

	void Application::OnEvent(Event& e)
	{
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<WindowCloseEvent>(BIND_EVENT_FN(Application::OnWindowClosed));

		for (auto it = m_LayerStack.end(); it != m_LayerStack.begin();)
		{
			(*--it)->OnEvent(e);
			if (e.Handled)
				break;

			if (e.GetEventType() == EventType::KeyPressed)
			{
				KeyPressedEvent& event = dynamic_cast<KeyPressedEvent&>(e);
				
				if (event.GetKeyCode() == KBR_KEY_W)
				{
					m_Camera.SetPosition(m_Camera.GetPosition() + glm::vec3(0.0f, 0.05f, 0.0f));
				}
				else if (event.GetKeyCode() == KBR_KEY_A)
				{
					m_Camera.SetPosition(m_Camera.GetPosition() + glm::vec3(-0.05f, 0.0f, 0.0f));
				}
				else if (event.GetKeyCode() == KBR_KEY_S)
				{
					m_Camera.SetPosition(m_Camera.GetPosition() + glm::vec3(0.0f, -0.05f, 0.0f));
				}
				else if (event.GetKeyCode() == KBR_KEY_D)
				{
					m_Camera.SetPosition(m_Camera.GetPosition() + glm::vec3(0.05f, 0.0f, 0.0f));
				}
				else if (event.GetKeyCode() == KBR_KEY_Q)
				{
					m_Camera.SetRotation(m_Camera.GetRotation() + 1.0f);
				}
				else if (event.GetKeyCode() == KBR_KEY_E)
				{
					m_Camera.SetRotation(m_Camera.GetRotation() - 1.0f);
				}
			}
		}
	}

	void Application::PushLayer(Layer* layer)
	{
		m_LayerStack.PushLayer(layer);
	}

	void Application::PushOverlay(Layer* overlay)
	{
		m_LayerStack.PushOverlay(overlay);
	}

	bool Application::OnWindowClosed(const WindowCloseEvent& e)
	{
		m_Running = false;
		return true;
	}
}
