#include "kbrpch.h"
#include "Application.h"

#include "GLFW/glfw3.h"


namespace Kerberos
{
#define BIND_EVENT_FN(x) std::bind(&x, this, std::placeholders::_1)

	Application::Application() 
	{
		m_Window = std::unique_ptr<Window>(Window::Create());
		m_Window->SetEventCallback(BIND_EVENT_FN(Application::OnEvent));
	}

	Application::~Application() = default;

	void Application::Run() const {
		const WindowResizeEvent e(1280, 720);
		KBR_CORE_TRACE(e.ToString());

		while (m_Running) 
		{
			m_Window->OnUpdate();
		}
	}

	void Application::OnEvent(Event& e)
	{
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<WindowCloseEvent>(BIND_EVENT_FN(Application::OnWindowClosed));

		KBR_CORE_TRACE("{0}", e.ToString());
	}

	bool Application::OnWindowClosed(const WindowCloseEvent& e) 
	{
		m_Running = false;
		return true;
	}
}
