#pragma once

#include "Kerberos/Renderer/GraphicsContext.h"

#include <GLFW/glfw3.h>

/**
 * Construct an OpenGLContext for the given GLFW window handle.
 * @param windowHandle Pointer to the GLFW window associated with this context.
 */

/**
 * Initialize the OpenGL context for the associated GLFW window and prepare any required OpenGL state.
 */

/**
 * Present the currently rendered frame by swapping the window's front and back buffers.
 */

/**
 * Query and cache device and compute-related capabilities (e.g., compute shader limits, work group sizes).
 */
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
