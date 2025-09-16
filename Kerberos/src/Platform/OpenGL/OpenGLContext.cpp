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
		KBR_PROFILE_FUNCTION();

		glfwMakeContextCurrent(m_WindowHandle);

		const int status = gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress));
		KBR_CORE_ASSERT(status, "Failed to initialize Glad!");
		KBR_CORE_INFO("Loaded OpenGL");

		KBR_CORE_INFO("OpenGL Info:");
		KBR_CORE_INFO("  Vendor: {0}", reinterpret_cast<const char*>(glGetString(GL_VENDOR)));
		KBR_CORE_INFO("  Renderer: {0}", reinterpret_cast<const char*>(glGetString(GL_RENDERER)));
		KBR_CORE_INFO("  Version: {0}", reinterpret_cast<const char*>(glGetString(GL_VERSION)));
	}

	void OpenGLContext::SwapBuffers() 
	{
		KBR_PROFILE_FUNCTION();

		glfwSwapBuffers(m_WindowHandle);
	}
}

