#include "kbrpch.h"
#include "OpenGLBuffer.h"

#include <glad/glad.h>

namespace Kerberos
{
	/// --------- Vertex Buffer --------- ///

	OpenGLVertexBuffer::OpenGLVertexBuffer(const float* vertices, const uint32_t size)
	{
		KBR_PROFILE_FUNCTION();

		glGenBuffers(1, &m_RendererID);
		glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);

		// TODO: Specify the buffer usage as a parameter
		glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW);

		/// Assuming each vertex has 3 components (x, y, z)
		m_Count = size / sizeof(float) / 3;
	}

	OpenGLVertexBuffer::OpenGLVertexBuffer(const uint32_t size) 
	{
		KBR_PROFILE_FUNCTION();

		glGenBuffers(1, &m_RendererID);
		glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);

		glBufferData(GL_ARRAY_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);
	}

	OpenGLVertexBuffer::~OpenGLVertexBuffer() 
	{
		KBR_PROFILE_FUNCTION();

		glDeleteBuffers(1, &m_RendererID);
	}

	void OpenGLVertexBuffer::SetData(const void* data, const uint32_t size)
	{
		KBR_PROFILE_FUNCTION();

		glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
		glBufferSubData(GL_ARRAY_BUFFER, 0, size, data);

		/// Assuming each vertex has 3 components (x, y, z)
		m_Count = size / sizeof(float) / 3;
	}

	void OpenGLVertexBuffer::Bind() const 
	{
		KBR_PROFILE_FUNCTION();

		glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
	}

	void OpenGLVertexBuffer::Unbind() const 
	{
		KBR_PROFILE_FUNCTION();

		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	/// --------- Index Buffer --------- ///

	OpenGLIndexBuffer::OpenGLIndexBuffer(const uint32_t* indices, const uint32_t count) 
		: m_Count(count)
	{
		KBR_PROFILE_FUNCTION();

		glGenBuffers(1, &m_RendererID);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererID);

		// TODO: Specify the buffer usage as a parameter
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizeiptr>(count * sizeof(uint32_t)), indices, GL_STATIC_DRAW);
	}

	OpenGLIndexBuffer::~OpenGLIndexBuffer() 
	{
		KBR_PROFILE_FUNCTION();

		glDeleteBuffers(1, &m_RendererID);
	}

	void OpenGLIndexBuffer::Bind() const 
	{
		KBR_PROFILE_FUNCTION();

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererID);
	}

	void OpenGLIndexBuffer::Unbind() const 
	{
		KBR_PROFILE_FUNCTION();

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}
}
