#pragma once

#include "Kerberos/Renderer/Framebuffer.h"

#include <vulkan/vulkan.h>

namespace Kerberos
{
	class VulkanFramebuffer final : public Framebuffer
	{
	public:
		explicit VulkanFramebuffer(const FramebufferSpecification& spec);
		~VulkanFramebuffer() override = default;

		void Invalidate();

		void Bind() override;
		void Unbind() override;

		void Resize(uint32_t width, uint32_t height) override;

		uint64_t GetColorAttachmentRendererID(uint32_t index = 0) const override;

		FramebufferSpecification& GetSpecification() override { return m_Specification; }
		const FramebufferSpecification& GetSpecification() const override { return m_Specification; }

	private:
		void ReleaseResources() const;

	private:
		FramebufferSpecification m_Specification;

		std::vector<FramebufferTextureSpecification> m_ColorAttachmentSpecs;
		FramebufferTextureSpecification m_DepthAttachmentSpec = FramebufferTextureFormat::None;
	};
}