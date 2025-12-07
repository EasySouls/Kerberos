#include "kbrpch.h"
#include "OpenGLContext.h"
#include "Kerberos/Core.h"

#include <glad/glad.h>

namespace Kerberos
{
	/**
	 * @brief Constructs an OpenGLContext bound to the given GLFW window.
	 *
	 * Associates this OpenGLContext with the provided GLFWwindow pointer so the context can be made current and used for GL operations.
	 *
	 * @param windowHandle Pointer to the GLFWwindow whose OpenGL context will be managed. Must not be null.
	 *
	 * The constructor asserts that `windowHandle` is not null. */
	OpenGLContext::OpenGLContext(GLFWwindow* windowHandle)
		: m_WindowHandle(windowHandle)
	{
		KBR_CORE_ASSERT(windowHandle, "Window handle is null!");
	}

	/**
	 * @brief Initializes the OpenGL context associated with the stored GLFW window.
	 *
	 * Makes the window's OpenGL context current, loads GL function pointers, queries and stores
	 * compute shader limits, and logs vendor, renderer, OpenGL and GLSL version information
	 * along with the retrieved compute shader limits.
	 *
	 * @note This function will assert if the GL loader (Glad) fails to initialize.
	 */
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

	/**
	 * @brief Presents the rendered frame by swapping the window's front and back buffers.
	 *
	 * Swaps the associated GLFW window's back buffer to the front so the most recently
	 * rendered framebuffer becomes visible on screen.
	 */
	void OpenGLContext::SwapBuffers()
	{
		KBR_PROFILE_FUNCTION();

		glfwSwapBuffers(m_WindowHandle);
	}

	/**
	 * @brief Queries and stores GPU compute shader work-group limits.
	 *
	 * Retrieves the per-dimension limits for compute shader dispatch from OpenGL
	 * and writes them into the OpenGLContext's m_ComputeInfo structure. Specifically,
	 * it populates:
	 * - m_ComputeInfo.MaxWorkGroupCount  (GL_MAX_COMPUTE_WORK_GROUP_COUNT)
	 * - m_ComputeInfo.MaxWorkGroupSize   (GL_MAX_COMPUTE_WORK_GROUP_SIZE)
	 * - m_ComputeInfo.MaxWorkGroupInvocations (GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS)
	 *
	 * Each stored value is a glm::vec3 containing the x, y, and z components as
	 * unsigned integers.
	 */
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
