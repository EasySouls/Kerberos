#include "kbrpch.h"
#include "OpenGLUniformBuffer.h"

#include <glad/glad.h>

namespace Kerberos
{
	OpenGLUniformBuffer::OpenGLUniformBuffer(const uint32_t size, const uint32_t binding) 
	{
		// TODO: Specify the buffer usage as a parameter
		glCreateBuffers(1, &m_RendererID);
		glNamedBufferData(m_RendererID, size, nullptr, GL_DYNAMIC_DRAW);
		glBindBufferBase(GL_UNIFORM_BUFFER, binding, m_RendererID);
	}

	OpenGLUniformBuffer::~OpenGLUniformBuffer()
	{
		glDeleteBuffers(1, &m_RendererID);
	}


	void OpenGLUniformBuffer::SetData(const void* data, const uint32_t size, const uint32_t offset)
	{
		glNamedBufferSubData(m_RendererID, offset, size, data);
	}

	void OpenGLUniformBuffer::SetDebugName(const std::string& debugName) 
	{
		glObjectLabel(GL_BUFFER, m_RendererID, static_cast<int>(debugName.size()), debugName.c_str());
	}
}
