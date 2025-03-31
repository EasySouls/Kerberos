#pragma once

#include "Kerberos/Renderer/Framebuffer.h"

namespace Kerberos
{
	class OpenGLFramebuffer : public Framebuffer
	{
	public:
		explicit OpenGLFramebuffer(const FramebufferSpecification& spec);
		~OpenGLFramebuffer() override = default;

		void Invalidate();

		void Bind() override;
		void Unbind() override;

		//virtual void Resize(uint32_t width, uint32_t height) override;

		uint32_t GetColorAttachmentRendererID() const override { return m_ColorAttachment; }
		FramebufferSpecification& GetSpecification() override { return m_Specification; }
		const FramebufferSpecification& GetSpecification() const override { return m_Specification; }

	private:
		uint32_t m_RendererID = 0;

		uint32_t m_ColorAttachment = 0;
		uint32_t m_DepthAttachment = 0;

		FramebufferSpecification m_Specification;
	};
}
