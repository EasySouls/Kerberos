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
		KBR_CORE_INFO("  Vendor:			{0}", reinterpret_cast<const char*>(glGetString(GL_VENDOR)));
		KBR_CORE_INFO("  Renderer:			{0}", reinterpret_cast<const char*>(glGetString(GL_RENDERER)));
		KBR_CORE_INFO("  Version OpenGL:	{0}", reinterpret_cast<const char*>(glGetString(GL_VERSION)));
		KBR_CORE_INFO("  Version GLSL:		{0}", reinterpret_cast<const char*>(glGetString(GL_SHADING_LANGUAGE_VERSION)));

		QueryComputeInfo();

		KBR_CORE_INFO("Compute Shader Limits:");
		KBR_CORE_INFO("  Max Compute Work Group Count: {0}, {1}, {2}",
					  m_ComputeInfo.MaxWorkGroupCount.x,
					  m_ComputeInfo.MaxWorkGroupCount.y,
					  m_ComputeInfo.MaxWorkGroupCount.z);

		KBR_CORE_INFO("  Max Compute Work Group Size: {0}, {1}, {2}",
					  m_ComputeInfo.MaxWorkGroupSize.x,
					  m_ComputeInfo.MaxWorkGroupSize.y,
					  m_ComputeInfo.MaxWorkGroupSize.z);

		KBR_CORE_INFO("  Max Compute Work Group Invocations: {0}, {1}, {2}",
					  m_ComputeInfo.MaxWorkGroupInvocations.x,
					  m_ComputeInfo.MaxWorkGroupInvocations.y,
					  m_ComputeInfo.MaxWorkGroupInvocations.z);
	}

	void OpenGLContext::SwapBuffers()
	{
		KBR_PROFILE_FUNCTION();

		glfwSwapBuffers(m_WindowHandle);
	}

	void OpenGLContext::QueryComputeInfo()
	{
		int maxWorkGroupCount[3];
		glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, &maxWorkGroupCount[0]);
		glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 1, &maxWorkGroupCount[1]);
		glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 2, &maxWorkGroupCount[2]);

		m_ComputeInfo.MaxWorkGroupCount = glm::vec3(
			static_cast<uint32_t>(maxWorkGroupCount[0]),
			static_cast<uint32_t>(maxWorkGroupCount[1]),
			static_cast<uint32_t>(maxWorkGroupCount[2])
		);

		int maxWorkGroupSize[3];
		glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0, &maxWorkGroupSize[0]);
		glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 1, &maxWorkGroupSize[1]);
		glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 2, &maxWorkGroupSize[2]);

		m_ComputeInfo.MaxWorkGroupSize = glm::vec3(
			static_cast<uint32_t>(maxWorkGroupSize[0]),
			static_cast<uint32_t>(maxWorkGroupSize[1]),
			static_cast<uint32_t>(maxWorkGroupSize[2])
		);

		int maxWorkGroupInvocations[3];
		glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, 0, &maxWorkGroupInvocations[0]);
		glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, 1, &maxWorkGroupInvocations[1]);
		glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, 2, &maxWorkGroupInvocations[2]);

		m_ComputeInfo.MaxWorkGroupInvocations = glm::vec3(
			static_cast<uint32_t>(maxWorkGroupInvocations[0]),
			static_cast<uint32_t>(maxWorkGroupInvocations[1]),
			static_cast<uint32_t>(maxWorkGroupInvocations[2])
		);
	}
}

