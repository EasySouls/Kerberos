#include "kbrpch.h"
#include "OpenGLFramebuffer.h"

#include <glad/glad.h>

namespace Kerberos
{
	namespace Utils
	{
		static bool IsDepthFormat(const FramebufferTextureFormat format)
		{
			switch (format)
			{
			case FramebufferTextureFormat::DEPTH24STENCIL8:
				return true;
			case FramebufferTextureFormat::None:
			case FramebufferTextureFormat::RGBA8:
			case FramebufferTextureFormat::RED_INTEGER:
				return false;
			}

			KBR_CORE_ASSERT(false, "Unknown framebuffer texture format!");
			return false;
		}

		static GLenum TextureTarget(const bool multisampled)
		{
			return multisampled ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D;
		}

		static void CreateTextures(const bool multisampled, uint32_t* outID, const size_t count)
		{
			glCreateTextures(TextureTarget(multisampled), static_cast<int>(count), outID);
		}

		static void BindTexture(const bool multisampled, const uint32_t id)
		{
			glBindTexture(TextureTarget(multisampled), id);
		}

		static void AttachColorTexture(const uint32_t id, const uint32_t samples, const GLint internalFormat, const GLenum format, const uint32_t width, const uint32_t height, const int index)
		{
			const int texWidth = static_cast<int>(width);
			const int texHeight = static_cast<int>(height);
			const int texSamples = static_cast<int>(samples);

			const bool multisampled = samples > 1;

			if (multisampled)
			{
				glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, texSamples, internalFormat, texWidth, texHeight, GL_FALSE);
			}
			else
			{
				glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, texWidth, texHeight, 0, format, GL_UNSIGNED_BYTE, nullptr);

				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

			}

			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + index, TextureTarget(multisampled), id, 0);
		}

		static void AttachDepthTexture(const uint32_t id, const uint32_t samples, const GLint format, const GLint attachmentType, const uint32_t width, const uint32_t height)
		{
			const int texWidth = static_cast<int>(width);
			const int texHeight = static_cast<int>(height);
			const int texSamples = static_cast<int>(samples);

			const bool multisampled = samples > 1;

			if (multisampled)
			{
				glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, texSamples, format, texWidth, texHeight, GL_FALSE);
			}
			else
			{
				glTexStorage2D(GL_TEXTURE_2D, 1, format, texWidth, texHeight);

				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

			}

			glFramebufferTexture2D(GL_FRAMEBUFFER, attachmentType, TextureTarget(multisampled), id, 0);
		}

		static GLenum ToGLFormat(const FramebufferTextureFormat format)
		{
			switch (format)
			{
			case FramebufferTextureFormat::RGBA8:
				return GL_RGBA8;
			case FramebufferTextureFormat::RED_INTEGER:
				return GL_RED_INTEGER;
			default:
				KBR_CORE_ASSERT(false, "This Kerbertos format doesn't have an opengl format equivalent");
				break;
			}

			return -1;
		}

		static GLenum ToGLDataType(const FramebufferTextureFormat format)
		{
			switch (format)
			{
			case FramebufferTextureFormat::RGBA8:
				return GL_UNSIGNED_BYTE;
			case FramebufferTextureFormat::RED_INTEGER:
				return GL_RED_INTEGER;
			default:
				KBR_CORE_ASSERT(false, "This Kerbertos format doesn't have an opengl format equivalent");
				break;
			}

			return -1;
		}
	}

	OpenGLFramebuffer::OpenGLFramebuffer(const FramebufferSpecification& spec) 
		: m_Specification(spec)
	{
		for (auto& format : spec.Attachments.Attachments)
		{
			if (Utils::IsDepthFormat(format.TextureFormat))
			{
				m_DepthAttachmentSpec = format;
			}
			else
			{
				m_ColorAttachmentSpecs.emplace_back(format);
			}
		}

		Invalidate();
	}

	OpenGLFramebuffer::~OpenGLFramebuffer() 
	{
		glDeleteFramebuffers(1, &m_RendererID);
		glDeleteTextures(static_cast<int>(m_ColorAttachments.size()), m_ColorAttachments.data());
		//glDeleteRenderbuffers(1, &m_DepthAttachment);
		glDeleteTextures(1, &m_DepthAttachment);
	}

	void OpenGLFramebuffer::Invalidate() 
	{
		/// Check if the framebuffer is already created
		if (m_RendererID)
		{
			glDeleteFramebuffers(1, &m_RendererID);
			glDeleteTextures(static_cast<int>(m_ColorAttachments.size()), m_ColorAttachments.data());
			//glDeleteRenderbuffers(1, &m_DepthAttachment);
			glDeleteTextures(1, &m_DepthAttachment);

			m_ColorAttachments.clear();
			m_DepthAttachment = 0;
		}

		// Create and bind the framebuffer
		glCreateFramebuffers(1, &m_RendererID);
		glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);

		/// Attachments

		const bool multisample = m_Specification.Samples > 1;

		/// Setup the color attachments
		if (!m_ColorAttachmentSpecs.empty())
		{
			m_ColorAttachments.resize(m_ColorAttachmentSpecs.size());
			/// Create all the color attachments and fill the vector with their IDs
			Utils::CreateTextures(multisample, m_ColorAttachments.data(), m_ColorAttachments.size());

			for (int i = 0; i < m_ColorAttachmentSpecs.size(); ++i)
			{
				Utils::BindTexture(multisample, m_ColorAttachments[i]);

				switch (m_ColorAttachmentSpecs[i].TextureFormat)
				{
					case FramebufferTextureFormat::RGBA8:
					{
						Utils::AttachColorTexture(m_ColorAttachments[i], m_Specification.Samples, GL_RGBA8, GL_RGBA,  m_Specification.Width, m_Specification.Height, i);
						break;
					}
					case FramebufferTextureFormat::RED_INTEGER:
					{
						Utils::AttachColorTexture(m_ColorAttachments[i], m_Specification.Samples, GL_R32I, GL_RED_INTEGER, m_Specification.Width, m_Specification.Height, i);
						break;
					}
					default:
						KBR_CORE_ASSERT(false, "Unknown framebuffer texture format!");
				}
			}
		}

		/// Setup the depth attachment if needed
		if (m_DepthAttachmentSpec.TextureFormat != FramebufferTextureFormat::None)
		{
			Utils::CreateTextures(multisample, &m_DepthAttachment, 1);
			Utils::BindTexture(multisample, m_DepthAttachment);
			switch (m_DepthAttachmentSpec.TextureFormat)
			{
				case FramebufferTextureFormat::DEPTH24STENCIL8:
				{
					Utils::AttachDepthTexture(m_DepthAttachment, m_Specification.Samples, GL_DEPTH24_STENCIL8, GL_DEPTH_STENCIL_ATTACHMENT, m_Specification.Width, m_Specification.Height);
					break;
				}
				default:
					KBR_CORE_ASSERT(false, "Unknown framebuffer texture format!");
			}
		}

		if (m_ColorAttachments.size() > 1)
		{
			KBR_CORE_ASSERT(m_ColorAttachments.size() <= 4, "Kerberos only supports up to 4 color attachments!");

			constexpr GLenum buffers[4] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };
			glDrawBuffers(static_cast<GLsizei>(m_ColorAttachments.size()), buffers);
		}
		else if (m_ColorAttachments.empty())
		{
			/// If this framebuffer is only used for a depth pass
			glDrawBuffer(GL_NONE);
			glReadBuffer(GL_NONE);
		}

		/// This is not used anymore, but kept for reference
		/// I still have to look up if creating depth buffers as render buffers is faster than textures

		//// Create a color attachment texture
		//glCreateTextures(GL_TEXTURE_2D, 1, &m_ColorAttachment);
		//glBindTexture(GL_TEXTURE_2D, m_ColorAttachment);

		//// Allocate storage for the texture
		//const int width = static_cast<int>(m_Specification.Width);
		//const int height = static_cast<int>(m_Specification.Height);
		//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

		//// Set the filtering parameters
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		//// Attach the color attachment texture to the framebuffer
		//glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_ColorAttachment, 0);

		//// Create a depth/stencil attachment renderbuffer
		//glCreateRenderbuffers(1, &m_DepthAttachment);
		//glBindRenderbuffer(GL_RENDERBUFFER, m_DepthAttachment);

		//// Allocate storage for the renderbuffer
		//glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);

		//// Attach the depth/stencil attachment renderbuffer to the framebuffer
		//glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_DepthAttachment);

		// Check if the framebuffer is complete
		KBR_CORE_ASSERT(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "Framebuffer is incomplete!");

		// Unbind the framebuffer
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void OpenGLFramebuffer::Bind() 
	{
		glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);
		glViewport(0, 0, static_cast<int>(m_Specification.Width), static_cast<int>(m_Specification.Height));
	}

	void OpenGLFramebuffer::Unbind() 
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void OpenGLFramebuffer::Resize(uint32_t width, uint32_t height) 
	{
		if (width == 0 || height == 0)
		{
			KBR_CORE_WARN("Attempted to resize framebuffer to {0}, {1}", width, height);
			return;
		}

		m_Specification.Width = width;
		m_Specification.Height = height;

		Invalidate();
	}

	int OpenGLFramebuffer::ReadPixel(const uint32_t attachmentIndex, const int x, const int y) 
	{
		KBR_CORE_ASSERT(attachmentIndex < m_ColorAttachments.size(), "attachmenIndex is out of bounds");

		glReadBuffer(GL_COLOR_ATTACHMENT0 + attachmentIndex);

		int pixelData;
		glReadPixels(x, y, 1, 1, GL_RED_INTEGER, GL_INT, &pixelData);
		return pixelData;
	}

	void OpenGLFramebuffer::ClearAttachment(const uint32_t attachmentIndex, const int value) 
	{
		KBR_CORE_ASSERT(attachmentIndex < m_ColorAttachments.size(), "attachmenIndex is out of bounds");

		const auto& spec = m_ColorAttachmentSpecs[attachmentIndex];

		glClearTexImage(m_ColorAttachments[attachmentIndex], 0, Utils::ToGLFormat(spec.TextureFormat), GL_INT, &value);
	}

	void OpenGLFramebuffer::ClearDepthAttachment(const int value) const 
	{
		KBR_CORE_ASSERT(m_DepthAttachment != 0, "Depth attachment is not set!");

		//glClearTexImage(m_DepthAttachment, 0, GL_DEPTH_STENCIL, GL_INT, &value);
		glClearBufferfi(GL_DEPTH_STENCIL, 0, static_cast<float>(value), 0);

		//KBR_CORE_ASSERT(glGetError() == GL_NO_ERROR, "Failed to clear depth attachment!");
	}
}
