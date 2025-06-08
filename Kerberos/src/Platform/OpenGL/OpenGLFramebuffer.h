#pragma once

#include "Kerberos/Renderer/Framebuffer.h"

namespace Kerberos
{
	class OpenGLFramebuffer : public Framebuffer
	{
	public:
		using RendererID = uint32_t;

		explicit OpenGLFramebuffer(const FramebufferSpecification& spec);
		~OpenGLFramebuffer() override;

		void Invalidate();

		void Bind() override;
		void Unbind() override;

		void Resize(uint32_t width, uint32_t height) override;

		int ReadPixel(uint32_t attachmentIndex, int x, int y) override;

		uint64_t GetColorAttachmentRendererID(const uint32_t index = 0) const override 
		{
			KBR_CORE_ASSERT(index < m_ColorAttachments.size(), "Index out of bounds!");
			return m_ColorAttachments.at(index);
		}

		FramebufferSpecification& GetSpecification() override { return m_Specification; }
		const FramebufferSpecification& GetSpecification() const override { return m_Specification; }

	private:
		RendererID m_RendererID = 0;

		FramebufferSpecification m_Specification;

		std::vector<FramebufferTextureSpecification> m_ColorAttachmentSpecs;
		FramebufferTextureSpecification m_DepthAttachmentSpec = FramebufferTextureFormat::None;

		std::vector<RendererID> m_ColorAttachments;
		RendererID m_DepthAttachment = 0;
	};
}
