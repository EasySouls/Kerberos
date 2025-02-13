#pragma once

#include "Core.h"
#include "Window.h"

namespace Kerberos
{
	class KERBEROS_API Application
	{
	public:
		Application();
		virtual ~Application();

		void Run();

	private:
		bool m_Running = true;
		std::unique_ptr<Window> m_Window;
	};

	// To be defined in client
	Application* CreateApplication();

}

