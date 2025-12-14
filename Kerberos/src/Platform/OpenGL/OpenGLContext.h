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
		void Render() override;
		void Present() override;

		void SetVSync(bool enabled) override;

	private:
		void QueryComputeInfo();

	private:
		GLFWwindow* m_WindowHandle;
	};

}

