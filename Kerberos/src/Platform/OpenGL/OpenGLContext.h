#pragma once

#include "Kerberos/Renderer/GraphicsContext.h"

#include <GLFW/glfw3.h>

namespace Kerberos
{
	class OpenGLContext final : public GraphicsContext
	{
	public:
		explicit OpenGLContext(GLFWwindow* windowHandle);

		void Init() override;
		void SwapBuffers() override;

	private:
		void QueryComputeInfo();

	private:
		GLFWwindow* m_WindowHandle;
	};

}

