#include "kbrpch.h"
#include "VulkanCommandList.h"

#include "VulkanContext.h"

namespace Kerberos
{
	VulkanCommandList::VulkanCommandList()
	{
		const VulkanContext& context = VulkanContext::Get();
		
		m_CommandBuffer = context.GetCurrentCommandBuffer();
	}

	void VulkanCommandList::Begin()
	{
		constexpr VkCommandBufferBeginInfo beginInfo{
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
			.pNext = nullptr,
			.flags = 0, // = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
			.pInheritanceInfo = nullptr
		};

		if (const VkResult result = vkBeginCommandBuffer(m_CommandBuffer, &beginInfo); result != VK_SUCCESS)
		{
			KBR_CORE_ASSERT(false, "Failed to begin command buffer! Result: {0}", VulkanHelpers::VkResultToString(result));
		}
	}

	void VulkanCommandList::End()
	{
		if (const VkResult result = vkEndCommandBuffer(m_CommandBuffer); result != VK_SUCCESS)
		{
			KBR_CORE_ASSERT(false, "Failed to record command buffer! Result: {0}", VulkanHelpers::VkResultToString(result));
		}
	}

	void VulkanCommandList::SetViewport(const uint32_t x, const uint32_t y, const uint32_t width, const uint32_t height) 
	{
		VkViewport viewport;
		viewport.x = static_cast<float>(x);
		viewport.y = static_cast<float>(y);
		viewport.width = static_cast<float>(width);
		viewport.height = static_cast<float>(height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		vkCmdSetViewport(m_CommandBuffer, 0, 1, &viewport);
	}

	void VulkanCommandList::SetScissorRect(const uint32_t x, const uint32_t y, const uint32_t width, const uint32_t height) 
	{
		VkRect2D scissor;
		scissor.offset = { .x = static_cast<int32_t>(x), .y = static_cast<int32_t>(y) };
		scissor.extent = { .width = width, .height = height };

		vkCmdSetScissor(m_CommandBuffer, 0, 1, &scissor);
	}

	void VulkanCommandList::BindPipeline(const Ref<GraphicsPipeline>& pipeline) 
	{
		vkCmdBindPipeline(m_CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->As<VulkanGraphicsPipeline>().GetVkPipeline());
	}

	void VulkanCommandList::BindVertexBuffer(const Ref<VertexBuffer>& vertexBuffer, const uint32_t slot) 
	{
		const VkBuffer buffers[] = { vertexBuffer->As<VulkanVertexBuffer>().GetVkBuffer() };
		constexpr VkDeviceSize offsets[] = { 0 };

		vkCmdBindVertexBuffers(m_CommandBuffer, slot, 1, buffers, offsets);
	}

	void VulkanCommandList::BindIndexBuffer(const Ref<IndexBuffer>& indexBuffer) 
	{
		const VkBuffer buffer = indexBuffer->As<VulkanIndexBuffer>().GetVkBuffer();
		const VkIndexType indexType = static_cast<VkIndexType>(indexBuffer->As<VulkanIndexBuffer>().GetType());

		vkCmdBindIndexBuffer(m_CommandBuffer, buffer, 0, indexType);
	}

	void VulkanCommandList::DrawVertexCount(const uint32_t vertexCount, const uint32_t startVertexLocation) 
	{
		vkCmdDraw(m_CommandBuffer, vertexCount, 1, startVertexLocation, 0);
	}

	void VulkanCommandList::DrawIndexed(const uint32_t indexCount, const uint32_t startIndexLocation, const int32_t baseVertexLocation) 
	{
		vkCmdDrawIndexed(m_CommandBuffer, indexCount, 1, startIndexLocation, baseVertexLocation, 0);
	}
}
