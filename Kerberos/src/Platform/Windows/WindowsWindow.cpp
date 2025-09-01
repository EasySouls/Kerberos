#include "kbrpch.h"
#include "WindowsWindow.h"
#include "Kerberos/Log.h"
#include "Kerberos/Events/ApplicationEvent.h"
#include "Kerberos/Events/MouseEvent.h"
#include "Kerberos/Events/KeyEvent.h"
#include "Kerberos/Core.h"
#include "Kerberos/Renderer/RendererAPI.h"
#include "Platform/OpenGL/OpenGLContext.h"
#include "Platform/Vulkan/VulkanContext.h"
#include "Platform/D3D11/D3D11Context.h"

extern "C"
{
	__declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
	//​__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}

namespace Kerberos
{
	static bool s_GLFWInitialized = false;

	static void GLFWErrorCallback(const int error, const char* description)
	{
		KBR_CORE_ERROR("GLFW Error ({0}): {1}", error, description);
	}

	Scope<Window> Window::Create(const WindowProps& props)
	{
		return CreateScope<WindowsWindow>(props);
	}

	WindowsWindow::WindowsWindow(const WindowProps& props) 
	{
		KBR_PROFILE_FUNCTION();

		WindowsWindow::Init(props);
	}

	WindowsWindow::~WindowsWindow() 
	{
		KBR_PROFILE_FUNCTION();

		WindowsWindow::Shutdown();
	};

	void WindowsWindow::Init(const WindowProps& props) 
	{
		KBR_PROFILE_FUNCTION();

		m_Data.Title = props.Title;
		m_Data.Width = props.Width;
		m_Data.Height = props.Height;

		KBR_CORE_INFO("Creating window {0} ({1}, {2})", props.Title, props.Width, props.Height);

		if (!s_GLFWInitialized)
		{
			const int success = glfwInit();
			KBR_CORE_ASSERT(success, "Could not initialize GLFW!");

			glfwSetErrorCallback(GLFWErrorCallback);

			s_GLFWInitialized = true;
		}

		const auto context = RendererAPI::GetAPI();

		/// Every other renderer API except OpenGL requires GLFW_NO_API to be set
		if (context != RendererAPI::API::OpenGL)
		{
			glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		}

		//glfwWindowHint(GLFW_DEPTH_BITS, 32);
		if (props.Maximized)
		{
			glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);
		}

		m_Window = glfwCreateWindow(static_cast<int>(props.Width), static_cast<int>(props.Height), m_Data.Title.c_str(), nullptr, nullptr);

		switch (context)
		{
		case RendererAPI::API::OpenGL:
			m_Context = new OpenGLContext(m_Window);
			break;
		case RendererAPI::API::Vulkan:
			m_Context = new VulkanContext(m_Window);
			break;
		case RendererAPI::API::D3D11:
			m_Context = new D3D11Context(m_Window);
			break;
		case RendererAPI::API::D3D12:
			KBR_CORE_ASSERT(false, "D3D12 is currently not supported!");
			break;
		}
		m_Context->Init();

		glfwSetWindowUserPointer(m_Window, &m_Data);

		SetVSync(true);

		// Set GLFW callbacks

		glfwSetWindowSizeCallback(m_Window, [](GLFWwindow* window, const int width, const int height)
			{
				WindowData& data = *static_cast<WindowData*>(glfwGetWindowUserPointer(window));
				data.Width = width;
				data.Height = height;

				WindowResizeEvent event(width, height);

				data.EventCallback(event);
			});

		glfwSetWindowCloseCallback(m_Window, [](GLFWwindow* window)
			{
				WindowData& data = *static_cast<WindowData*>(glfwGetWindowUserPointer(window));
				WindowCloseEvent event;
				data.EventCallback(event);
			});

		glfwSetKeyCallback(m_Window, [](GLFWwindow* window, const int key, const int scancode, const int action, const int mods)
			{
				WindowData& data = *static_cast<WindowData*>(glfwGetWindowUserPointer(window));

				switch (action)
				{
					case GLFW_PRESS:
					{
						KeyPressedEvent event(key, 0);
						data.EventCallback(event);
						break;
					}

					case GLFW_RELEASE:
					{
						KeyReleasedEvent event(key);
						data.EventCallback(event);
						break;
					}

					case GLFW_REPEAT:
					{
						KeyPressedEvent event(key, 1);
						data.EventCallback(event);
						break;
					}

					default:
						break;
				}
			});

		glfwSetCharCallback(m_Window, [](GLFWwindow* window, const unsigned int keycode)
			{
				const WindowData& data = *static_cast<WindowData*>(glfwGetWindowUserPointer(window));
				KeyTypedEvent event(keycode);
				data.EventCallback(event);
			});

		glfwSetMouseButtonCallback(m_Window, [](GLFWwindow* window, const int button, const int action, const int mods)
			{
				const WindowData& data = *static_cast<WindowData*>(glfwGetWindowUserPointer(window));
				switch (action)
				{
					case GLFW_PRESS:
					{
						MouseButtonPressedEvent event(button);
						data.EventCallback(event);
						break;
					}
					case GLFW_RELEASE:
					{
						MouseButtonReleasedEvent event(button);
						data.EventCallback(event);
						break;
					}
					default:
						break;
				}
			});

		glfwSetScrollCallback(m_Window, [](GLFWwindow* window, const double xOffset, const double yOffset)
			{
				const WindowData& data = *static_cast<WindowData*>(glfwGetWindowUserPointer(window));
				MouseScrolledEvent event(static_cast<float>(xOffset), static_cast<float>(yOffset));
				data.EventCallback(event);
			});

		glfwSetCursorPosCallback(m_Window, [](GLFWwindow* window, const double xPos, const double yPos)
			{
				const WindowData& data = *static_cast<WindowData*>(glfwGetWindowUserPointer(window));
				MouseMovedEvent event(static_cast<float>(xPos), static_cast<float>(yPos));
				data.EventCallback(event);
			});

		glfwSetDropCallback(m_Window, [](GLFWwindow* window, const int pathCount, const char* paths[])
			{
				const WindowData& data = *static_cast<WindowData*>(glfwGetWindowUserPointer(window));

				std::vector<std::filesystem::path> filepaths(pathCount);
				for (int i = 0; i < pathCount; ++i)
				{
					filepaths[i] = paths[i];
				}

				WindowDropEvent event(std::move(filepaths));
				data.EventCallback(event);
			});
	}

	void WindowsWindow::Shutdown() 
	{
		KBR_PROFILE_FUNCTION();

		glfwDestroyWindow(m_Window);
	}

	void WindowsWindow::OnUpdate() 
	{
		KBR_PROFILE_FUNCTION();

		glfwPollEvents();
		m_Context->SwapBuffers();
	}

	void WindowsWindow::SetVSync(const bool enabled) 
	{
		KBR_PROFILE_FUNCTION();

		/// Ensure that VSync is only set for OpenGL, as other APIs have a different way of handling it
		if (RendererAPI::GetAPI() != RendererAPI::API::OpenGL)
			return;

		if (enabled)
		{
			glfwSwapInterval(1);
		}
		else
		{
			glfwSwapInterval(0);
		}
		m_Data.VSync = enabled;
	}

	bool WindowsWindow::IsVSync() const 
	{
		return m_Data.VSync;
	}
}
