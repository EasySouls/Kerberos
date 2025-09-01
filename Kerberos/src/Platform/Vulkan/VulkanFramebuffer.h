#pragma once

#include "Kerberos/Renderer/Framebuffer.h"

#include <vulkan/vulkan.h>

namespace Kerberos
{
	class VulkanFramebuffer final : public Framebuffer
	{
	public:
		explicit VulkanFramebuffer(FramebufferSpecification spec);
		~VulkanFramebuffer() override;

		void Invalidate();

		void Bind() override;
		void Unbind() override;

		void Resize(uint32_t width, uint32_t height) override;
		int ReadPixel(uint32_t attachmentIndex, int x, int y) override;

		void BindColorTexture(uint32_t slot, uint32_t index) const override;
		void BindDepthTexture(uint32_t slot) const override;

		void ClearAttachment(uint32_t attachmentIndex, int value) override;
		void ClearDepthAttachment(float value) const override;

		uint64_t GetColorAttachmentRendererID(uint32_t index = 0) const override;
		uint64_t GetDepthAttachmentRendererID() const override;

		FramebufferSpecification& GetSpecification() override { return m_Specification; }
		const FramebufferSpecification& GetSpecification() const override { return m_Specification; }

		void SetDebugName(const std::string& name) const override;

		VkRenderPass GetRenderPass() const { return m_RenderPass; }

	private:
		void ReleaseResources();

		static void CreateImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& memory);
		static void CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags imageAspectFlags, VkImageView& imageView);

		static std::vector<VkSubpassDependency> CreateSubpassDependencies();

	private:
		FramebufferSpecification m_Specification;
		glm::vec4 m_ClearColor;
		glm::vec4 m_DepthClearValue;

		std::vector<FramebufferTextureSpecification> m_ColorAttachmentSpecs;
		FramebufferTextureSpecification m_DepthAttachmentSpec = FramebufferTextureFormat::None;

		VkFramebuffer m_Framebuffer = VK_NULL_HANDLE;
		VkRenderPass m_RenderPass = VK_NULL_HANDLE;

		std::vector<VkImage> m_ColorAttachments;
		std::vector<VkImageView> m_ColorAttachmentViews;
		std::vector<VkDeviceMemory> m_ColorAttachmentMemories;

		VkImage m_DepthAttachment = VK_NULL_HANDLE;
		VkImageView m_DepthAttachmentView = VK_NULL_HANDLE;
		VkDeviceMemory m_DepthAttachmentMemory = VK_NULL_HANDLE;

		VkSampler m_ColorAttachmentSampler = VK_NULL_HANDLE;

		VkCommandPool m_CommandPool = VK_NULL_HANDLE;
		VkCommandBuffer m_CommandBuffer = VK_NULL_HANDLE;

		VkFence m_Fence = VK_NULL_HANDLE;

		std::vector<VkDescriptorSet> m_ColorAttachmentDescriptorSets;
		VkDescriptorSet m_DepthAttachmentDescriptorSet = VK_NULL_HANDLE;
	};
}