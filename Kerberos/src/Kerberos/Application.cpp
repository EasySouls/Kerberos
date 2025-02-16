#include "kbrpch.h"
#include "Application.h"
#include "Kerberos/Core.h"

#include <glad/glad.h>

namespace Kerberos
{
#define BIND_EVENT_FN(x) std::bind(&x, this, std::placeholders::_1)

	Application* Application::s_Instance = nullptr;

	static GLenum ShaderDataTypeToOpenGLBaseType(const ShaderDataType type)
	{
		switch (type)
		{
			case ShaderDataType::Float:
			case ShaderDataType::Float2:
			case ShaderDataType::Float3:
			case ShaderDataType::Float4:
			case ShaderDataType::Mat3:
			case ShaderDataType::Mat4:
				return GL_FLOAT;
			case ShaderDataType::Int:
			case ShaderDataType::Int2:
			case ShaderDataType::Int3:
			case ShaderDataType::Int4:
				return GL_INT;
			case ShaderDataType::Bool:
				return GL_BOOL;
		}

		KBR_ASSERT(false, "Unknown ShaderDataType");
		return 0;
	}

	Application::Application() 
	{
		KBR_CORE_ASSERT(!s_Instance, "Application already exists!");
		s_Instance = this;

		m_Window = std::unique_ptr<Window>(Window::Create());
		m_Window->SetEventCallback(BIND_EVENT_FN(Application::OnEvent));

		m_ImGuiLayer = new ImGuiLayer();
		PushOverlay(m_ImGuiLayer);

		glGenVertexArrays(1, &m_VertexArray);
		glBindVertexArray(m_VertexArray);

		constexpr float vertices[3 * 7] = {
			-0.5f, -0.5f, 0.0f, 0.9f, 0.3f, 1.0f, 1.0f,
			 0.5f, -0.5f, 0.0f, 0.9f, 0.3f, 1.0f, 1.0f,
			 0.0f,  0.5f, 0.0f, 0.9f, 0.3f, 1.0f, 1.0f,
		};

		m_VertexBuffer.reset(VertexBuffer::Create(vertices, sizeof(vertices)));

		{
			BufferLayout layout = {
				{ ShaderDataType::Float3, "a_Pos"  },
				{ ShaderDataType::Float4, "a_Color" }
			};

			m_VertexBuffer->SetLayout(layout);
		}

		const auto& layout = m_VertexBuffer->GetLayout();
		int index = 0;
		for (const auto& element : m_VertexBuffer->GetLayout())
		{
			glEnableVertexAttribArray(index);
			glVertexAttribPointer(index, 
				static_cast<int>(element.GetComponentCount()), 
				ShaderDataTypeToOpenGLBaseType(element.Type), 
				element.Normalized ? GL_TRUE : GL_FALSE, 
				static_cast<int>(layout.GetStride()),
				(const void*)element.Offset);

			index++;
		}

		constexpr uint32_t indices[3] = { 0, 1, 2 };

		m_IndexBuffer.reset(IndexBuffer::Create(indices, sizeof(indices) / sizeof(uint32_t)));

		const std::string vertexSrc = R"(
			#version 330 core
			
			layout(location=0) in vec3 a_Pos;
			layout(location=1) in vec4 a_Color;

			out vec3 v_Pos;
			out vec4 v_Color;

			void main() {
				v_Pos = a_Pos;
				v_Color = a_Color;
				gl_Position = vec4(a_Pos, 1.0);
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
	}

	Application::~Application() = default;

	void Application::Run() 
	{
		while (m_Running) 
		{
			glClearColor(0.1f, 0.1f, 0.1f, 1);
			glClear(GL_COLOR_BUFFER_BIT);

			m_Shader->Bind();
			glBindVertexArray(m_VertexArray);
			glDrawElements(GL_TRIANGLES, static_cast<int>(m_IndexBuffer->GetCount()), GL_UNSIGNED_INT, nullptr);

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
