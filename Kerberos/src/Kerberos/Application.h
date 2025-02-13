#pragma once

#include "Core.h"
#include "Window.h"
#include "Kerberos/Events/ApplicationEvent.h"

namespace Kerberos
{
	class KERBEROS_API Application
	{
	public:
		Application();
		virtual ~Application();

		void Run() const;

		void OnEvent(Event& e);

	private:
		bool OnWindowClosed(const WindowCloseEvent& e);

		bool m_Running = true;
		std::unique_ptr<Window> m_Window;
	};

	// To be defined in client
	Application* CreateApplication();

}

