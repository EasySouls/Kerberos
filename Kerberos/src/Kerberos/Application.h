#pragma once

#include "Core.h"

namespace Kerberos
{
	class KERBEROS_API Application
	{
	public:
		Application();
		virtual ~Application();

		void Run();
	};

	// To be defined in client
	Application* CreateApplication();

}

