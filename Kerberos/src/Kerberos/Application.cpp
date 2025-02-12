#include "kbrpch.h"
#include "Application.h"

#include "Log.h"
#include "Events/ApplicationEvent.h"


namespace Kerberos
{
	Application::Application() {}

	Application::~Application() {}
	
	void Application::Run() 
	{
		const WindowResizeEvent e(1280, 720);
		KBR_CORE_TRACE(e.ToString());

		while (true) {}
	}
}
