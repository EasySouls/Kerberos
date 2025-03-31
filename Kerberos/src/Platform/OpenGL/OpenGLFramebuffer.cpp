#include "kbrpch.h"
#include "OpenGLFramebuffer.h"

#include <glad/glad.h>

namespace Kerberos
{
	OpenGLFramebuffer::OpenGLFramebuffer(const FramebufferSpecification& spec) 
		: m_Specification(spec)
	{
		Invalidate();
	}

	void OpenGLFramebuffer::Invalidate() 
	{
		// Create and bind the framebuffer
		glCreateFramebuffers(1, &m_RendererID);
		glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);

		// Create a color attachment texture
		glCreateTextures(GL_TEXTURE_2D, 1, &m_ColorAttachment);
		glBindTexture(GL_TEXTURE_2D, m_ColorAttachment);

		// Allocate storage for the texture
		const int width = static_cast<int>(m_Specification.Width);
		const int height = static_cast<int>(m_Specification.Height);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

		// Set the filtering parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// Attach the color attachment texture to the framebuffer
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_ColorAttachment, 0);

		//glCreateTextures(GL_TEXTURE_2D, 1, &m_DepthAttachment);
		//glBindTexture(GL_TEXTURE_2D, m_DepthAttachment);
		//glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, width, height, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, nullptr);
		//glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, m_DepthAttachment, 0);

		// Create a depth/stencil attachment renderbuffer
		glCreateRenderbuffers(1, &m_DepthAttachment);
		glBindRenderbuffer(GL_RENDERBUFFER, m_DepthAttachment);

		// Allocate storage for the renderbuffer
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);

		// Attach the depth/stencil attachment renderbuffer to the framebuffer
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_DepthAttachment);

		// Check if the framebuffer is complete
		KBR_CORE_ASSERT(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "Framebuffer is incomplete!");

		// Unbind the framebuffer
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void OpenGLFramebuffer::Bind() 
	{
		glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);
	}

	void OpenGLFramebuffer::Unbind() 
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
}
