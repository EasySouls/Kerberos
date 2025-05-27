#pragma once

#include "Kerberos/Log.h"
#include "Kerberos/Renderer/Framebuffer.h"

namespace Kerberos
{
	class D3D11Framebuffer final : public Framebuffer
	{
	public:
		using RendererID = uint32_t;

		explicit D3D11Framebuffer(const FramebufferSpecification& spec);
		~D3D11Framebuffer() override = default;

		void Invalidate();

		void Bind() override;
		void Unbind() override;

		void Resize(uint32_t width, uint32_t height) override;

		RendererID GetColorAttachmentRendererID(const uint32_t index = 0) const override
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