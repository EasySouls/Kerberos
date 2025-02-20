#include "kbrpch.h"

#include "VulkanRendererAPI.h"

#include <vulkan/vulkan.h>

namespace Kerberos
{
	void VulkanRendererAPI::Init()
	{
	}

	void VulkanRendererAPI::SetupPipeline()
	{

	}

	void VulkanRendererAPI::SetClearColor(const glm::vec4& color)
	{
		// TODO: I am really not sure about this
		VkCommandBuffer m_CommandBuffer = VK_NULL_HANDLE;
		VkImage m_Image = VK_NULL_HANDLE;

		VkClearValue clearValue;
		clearValue.color = { { color.r, color.g, color.b, color.a } };

		VkRenderPassBeginInfo renderPassBeginInfo = {};
		renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;

		// Specify the clear values for attachments
		renderPassBeginInfo.pClearValues = &clearValue;
		renderPassBeginInfo.clearValueCount = 1; // If you have multiple attachments

		vkCmdBeginRenderPass(m_CommandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

		VkImageSubresourceRange imageSubresourceRange = {};
		imageSubresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageSubresourceRange.baseMipLevel = 0;
		imageSubresourceRange.levelCount = 1;
		imageSubresourceRange.baseArrayLayer = 0;
		imageSubresourceRange.layerCount = 1;

		vkCmdClearColorImage(
			m_CommandBuffer,
			m_Image,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, // Example layout
			&clearValue.color,
			1,
			&imageSubresourceRange
		);
	}

	void VulkanRendererAPI::Clear()
	{
		// TODO: I am really not sure about this

		VkCommandBuffer m_CommandBuffer = VK_NULL_HANDLE;

		std::vector<VkClearValue> clearValues(2);
		clearValues[0].color = { { 0.2f, 0.4f, 0.6f, 1.0f } };
		clearValues[1].depthStencil = { 1.0f, 0 };

		std::vector<VkClearAttachment> attachmentsToClear(2);
		attachmentsToClear[0].aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		attachmentsToClear[0].clearValue = clearValues[0];
		attachmentsToClear[0].colorAttachment = 0;

		attachmentsToClear[1].aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
		attachmentsToClear[1].clearValue = clearValues[1];
		attachmentsToClear[1].colorAttachment = 0;

		VkClearRect clearRect;
		clearRect.rect.offset = { 0, 0 };
		clearRect.rect.extent = { 100, 100 };
		clearRect.layerCount = 1;
		clearRect.baseArrayLayer = 0;

		vkCmdClearAttachments(
			m_CommandBuffer,
			static_cast<uint32_t>(attachmentsToClear.size()),
			attachmentsToClear.data(),
			1,
			&clearRect
		);
	}

	void VulkanRendererAPI::DrawIndexed(const Ref<VertexArray>& vertexArray)
	{
	}
}
