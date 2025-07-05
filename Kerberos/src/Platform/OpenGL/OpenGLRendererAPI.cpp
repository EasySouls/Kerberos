#include "kbrpch.h"
#include "OpenGLRendererAPI.h"

#include <glad/glad.h>

namespace Kerberos
{
	static void OpenGLMessageCallback(
		unsigned source,
		unsigned type,
		unsigned id,
		const unsigned severity,
		int length,
		const char* message,
		const void* userParam)
	{
		switch (severity)
		{
		case GL_DEBUG_SEVERITY_HIGH:         KBR_CORE_CRITICAL(message); return;
		case GL_DEBUG_SEVERITY_MEDIUM:       KBR_CORE_ERROR(message); return;
		case GL_DEBUG_SEVERITY_LOW:          KBR_CORE_WARN(message); return;
		case GL_DEBUG_SEVERITY_NOTIFICATION: KBR_CORE_TRACE(message); return;
		default: KBR_CORE_ASSERT(false, "Unknown severity level!");
		}
	}

	void OpenGLRendererAPI::Init() 
	{
	#ifdef KBR_DEBUG
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(OpenGLMessageCallback, nullptr);

		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, nullptr, GL_FALSE);
	#endif


		//glEnable(GL_BLEND);
		//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);

		/*glEnable(GL_POLYGON_OFFSET_FILL);
		glPolygonOffset(1.0f, 1.0f);*/
		//glEnable(GL_LINE_SMOOTH);

		//glEnable(GL_CULL_FACE);

		/*glEnable(GL_FRAMEBUFFER_SRGB);*/
	}

	void OpenGLRendererAPI::SetViewport(const uint32_t x, const uint32_t y, const uint32_t width,
		const uint32_t height) 
	{
		glViewport(static_cast<int>(x), static_cast<int>(y), static_cast<int>(width), static_cast<int>(height));
	}

	void OpenGLRendererAPI::SetClearColor(const glm::vec4& color) 
	{
		glClearColor(color.r, color.g, color.b, color.a);
	}

	void OpenGLRendererAPI::Clear()
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	void OpenGLRendererAPI::ClearDepth() 
	{
		glClear(GL_DEPTH_BUFFER_BIT);
	}

	void OpenGLRendererAPI::SetDepthTest(const bool enabled) 
	{
		if (enabled)
			glEnable(GL_DEPTH_TEST);
		else
			glDisable(GL_DEPTH_TEST);
	}

	void OpenGLRendererAPI::SetDepthFunc(const DepthFunc func)
	{
		GLenum glFunc = GL_LESS;
		switch (func)
		{
		case DepthFunc::Never:         glFunc = GL_NEVER; break;
		case DepthFunc::Always:        glFunc = GL_ALWAYS; break;
		case DepthFunc::Equal:         glFunc = GL_EQUAL; break;
		case DepthFunc::NotEqual:      glFunc = GL_NOTEQUAL; break;
		case DepthFunc::Less:          glFunc = GL_LESS; break;
		case DepthFunc::LessEqual:     glFunc = GL_LEQUAL; break;
		case DepthFunc::Greater:       glFunc = GL_GREATER; break;
		case DepthFunc::GreaterEqual:  glFunc = GL_GEQUAL; break;
		default:                       KBR_CORE_ASSERT(false, "Unknown depth function!"); break;
		}

		glDepthFunc(glFunc);
	}

	void OpenGLRendererAPI::DrawIndexed(const Ref<VertexArray>& vertexArray, const uint32_t indexCount)
	{
		vertexArray->Bind();

		const uint32_t count = indexCount ? indexCount : vertexArray->GetIndexBuffer()->GetCount();
		glDrawElements(GL_TRIANGLES, static_cast<int>(count), GL_UNSIGNED_INT, nullptr);
	}

	void OpenGLRendererAPI::DrawArray(const Ref<VertexArray>& vertexArray, const uint32_t vertexCount)
	{
		vertexArray->Bind();

		glDrawArrays(GL_TRIANGLES, 0, static_cast<int>(vertexCount));
	}
}
