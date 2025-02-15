#include "kbrpch.h"
#include "OpenGLContext.h"
#include "Kerberos/Core.h"

#include <glad/glad.h>

namespace Kerberos
{
	OpenGLContext::OpenGLContext(GLFWwindow* windowHandle) 
		: m_WindowHandle(windowHandle)
	{
		KBR_CORE_ASSERT(windowHandle, "Window handle is null!");
	}

	void OpenGLContext::Init() 
	{
		glfwMakeContextCurrent(m_WindowHandle);

		const int status = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
		KBR_CORE_ASSERT(status, "Failed to initialize Glad!");
		KBR_CORE_INFO("Loaded OpenGL");
	}

	void OpenGLContext::SwapBuffers() 
	{
		glfwSwapBuffers(m_WindowHandle);
	}
}

