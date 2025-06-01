#include "kbrpch.h"

#include "VulkanRendererAPI.h"

#include "VulkanContext.h"
#include "VulkanHelpers.h"
#include "VulkanShader.h"

namespace Kerberos
{
	void VulkanRendererAPI::Init()
	{
	}

	void VulkanRendererAPI::Cleanup() const
	{
	}

	void VulkanRendererAPI::SetViewport(const uint32_t x, const uint32_t y, const uint32_t width,
		const uint32_t height) 
	{
		VkViewport viewport = {};
		viewport.x = static_cast<float>(x);
		viewport.y = static_cast<float>(y);
		viewport.width = static_cast<float>(width);
		viewport.height = static_cast<float>(height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		const VkCommandBuffer commandBuffer = VulkanContext::Get().GetCommandBuffer();

		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
	}

	void VulkanRendererAPI::SetClearColor(const glm::vec4& color)
	{
		// TODO: I am really not sure about this

		const VulkanContext& context = VulkanContext::Get();
		const VkCommandBuffer commandBuffer = context.GetCommandBuffer();

		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		beginInfo.pNext = nullptr;

		vkBeginCommandBuffer(commandBuffer, &beginInfo);

		VkClearValue clearValue;
		clearValue.color = { { color.r, color.g, color.b, color.a } };

		const VkRenderPass& renderPass = context.GetRenderPass();
		const auto swapChainFramebuffers = context.GetSwapChainFramebuffers();

		VkRenderPassBeginInfo renderPassBeginInfo = {};
		renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;

		// Specify the clear values for attachments
		renderPassBeginInfo.pClearValues = &clearValue;
		renderPassBeginInfo.clearValueCount = 1; // If you have multiple attachments
		renderPassBeginInfo.renderPass = renderPass;
		renderPassBeginInfo.framebuffer = swapChainFramebuffers[0]; // Use the first framebuffer for now
		/// TODO: Handle multiple framebuffers properly

		vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

		VkImageSubresourceRange imageSubresourceRange = {};
		imageSubresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageSubresourceRange.baseMipLevel = 0;
		imageSubresourceRange.levelCount = 1;
		imageSubresourceRange.baseArrayLayer = 0;
		imageSubresourceRange.layerCount = 1;

		const std::vector<VkImage>& swapChainImages = VulkanContext::Get().GetSwapChainImages();
		for (const auto& image : swapChainImages)
		{
			// Transition the image layout to VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL before clearing
			VkImageMemoryBarrier barrier = {};
			barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			barrier.srcAccessMask = 0; // No access mask needed for clearing
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			barrier.image = image;
			barrier.subresourceRange = imageSubresourceRange;
			vkCmdPipelineBarrier(
				commandBuffer,
				VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
				VK_PIPELINE_STAGE_TRANSFER_BIT,
				0, // No flags
				0, nullptr, // No memory barriers
				0, nullptr, // No buffer barriers
				1, &barrier // Image barrier
			);

			vkCmdClearColorImage(
				commandBuffer,
				image,
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, // Example layout
				&clearValue.color,
				1,
				&imageSubresourceRange
			);
		}
		
	}

	void VulkanRendererAPI::Clear()
	{
		// TODO: I am really not sure about this

		const VkCommandBuffer commandBuffer = VulkanContext::Get().GetCommandBuffer();

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
			commandBuffer,
			static_cast<uint32_t>(attachmentsToClear.size()),
			attachmentsToClear.data(),
			1,
			&clearRect
		);
	}

	void VulkanRendererAPI::SetDepthTest(bool enabled) 
	{
		throw std::runtime_error("Depth test not implemented in VulkanRendererAPI yet!");
	}

	void VulkanRendererAPI::SetDepthFunc(DepthFunc func) 
	{
		throw std::runtime_error("Depth function not implemented in VulkanRendererAPI yet!");
	}

	void VulkanRendererAPI::DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t indexCount)
	{
	}

	void VulkanRendererAPI::DrawArray(const Ref<VertexArray>& vertexArray, uint32_t vertexCount) {}

	void VulkanRendererAPI::RecordCommandBuffer(const VkCommandBuffer commandBuffer, const uint32_t imageIndex) const
	{
		const VulkanContext& context = VulkanContext::Get();
		const VkRenderPass& renderPass = context.GetRenderPass();
		const std::vector<VkFramebuffer>& swapChainFramebuffers = context.GetSwapChainFramebuffers();
		const VkPipeline& graphicsPipeline = context.GetPipeline();

		constexpr VkCommandBufferBeginInfo beginInfo {
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		};

		if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to begin recording command buffer!");
		}

		VkClearValue clearColor = { {{0.0f, 0.0f, 0.0f, 1.0f}} };

		const VkExtent2D swapChainExtent = VulkanContext::Get().GetSwapChainExtent();

		const VkRenderPassBeginInfo renderPassInfo{
			.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
			.renderPass = renderPass,
			.framebuffer = swapChainFramebuffers[imageIndex],
			.renderArea = {
				.offset = { 0, 0 },
				.extent = swapChainExtent
			},
			.clearValueCount = 1,
			.pClearValues = &clearColor
		};

		vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

		VkViewport viewport;
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(swapChainExtent.width);
		viewport.height = static_cast<float>(swapChainExtent.height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

		VkRect2D scissor;
		scissor.offset = { 0, 0 };
		scissor.extent = swapChainExtent;
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

		vkCmdDraw(commandBuffer, 3, 1, 0, 0);

		vkCmdEndRenderPass(commandBuffer);

		if (const VkResult result = vkEndCommandBuffer(commandBuffer); result != VK_SUCCESS)
		{
			KBR_CORE_ASSERT(false, "Failed to record command buffer! Result: {0}", VulkanHelpers::VkResultToString(result));
			throw std::runtime_error("failed to record command buffer!");
		}
	}
}
