#include "kbrpch.h"
#include "Application.h"

#include "Log.h"
#include "Events/ApplicationEvent.h"


namespace Kerberos
{
	Application::Application() 
	{
		m_Window = std::unique_ptr<Window>(Window::Create());
	}

	Application::~Application() = default;

	void Application::Run() 
	{
		const WindowResizeEvent e(1280, 720);
		KBR_CORE_TRACE(e.ToString());

		while (m_Running) 
		{
			m_Window->OnUpdate();
		}
	}
}
