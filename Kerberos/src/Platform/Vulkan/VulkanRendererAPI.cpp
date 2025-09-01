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
		//VkViewport viewport = {};
		//viewport.x = static_cast<float>(x);
		//viewport.y = static_cast<float>(y);
		//viewport.width = static_cast<float>(width);
		//viewport.height = static_cast<float>(height);
		//viewport.minDepth = 0.0f;
		//viewport.maxDepth = 1.0f;

		//const VkCommandBuffer commandBuffer = VulkanContext::Get().GetCommandBuffers();

		//vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
	}

	void VulkanRendererAPI::SetClearColor(const glm::vec4& color)
	{
		m_ClearColor = color;
	}

	void VulkanRendererAPI::Clear()
	{
		// TODO: I am really not sure about this

		//const VkCommandBuffer commandBuffer = VulkanContext::Get().GetCommandBuffers();

		//VkCommandBufferBeginInfo cmdBeginInfo;
		//cmdBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		//cmdBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		//cmdBeginInfo.pNext = nullptr;
		//cmdBeginInfo.pInheritanceInfo = nullptr;

		//vkBeginCommandBuffer(commandBuffer, &cmdBeginInfo);

		//std::vector<VkClearValue> clearValues(2);
		//clearValues[0].color = { { 0.2f, 0.4f, 0.6f, 1.0f } };
		//clearValues[1].depthStencil = { 1.0f, 0 };

		//std::vector<VkClearAttachment> attachmentsToClear(2);
		//attachmentsToClear[0].aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		//attachmentsToClear[0].clearValue = clearValues[0];
		//attachmentsToClear[0].colorAttachment = 0;

		//attachmentsToClear[1].aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
		//attachmentsToClear[1].clearValue = clearValues[1];
		//attachmentsToClear[1].colorAttachment = 0;

		//VkClearRect clearRect;
		//clearRect.rect.offset = { 0, 0 };
		//clearRect.rect.extent = { 100, 100 };
		//clearRect.layerCount = 1;
		//clearRect.baseArrayLayer = 0;

		//vkCmdClearAttachments(
		//	commandBuffer,
		//	static_cast<uint32_t>(attachmentsToClear.size()),
		//	attachmentsToClear.data(),
		//	1,
		//	&clearRect
		//);

		//vkEndCommandBuffer(commandBuffer);
	}

	void VulkanRendererAPI::ClearDepth() 
	{

		const VkCommandBuffer commandBuffer = VulkanContext::Get().GetOneTimeCommandBuffer();
		
		/// TODO: Implement clearing depth buffer in Vulkan
	}

	void VulkanRendererAPI::SetDepthTest(bool enabled) 
	{
		/// TODO: Implement Pipeline class which has a baked-in depth testing, or maybe set in dynamically 
		//throw std::runtime_error("Depth test not implemented in VulkanRendererAPI yet!");
	}

	void VulkanRendererAPI::SetDepthFunc(DepthFunc func) 
	{
		/// TODO: Implement Pipeline class which has a baked-in depth func, or maybe set in dynamically 
		//throw std::runtime_error("Depth function not implemented in VulkanRendererAPI yet!");
	}

	void VulkanRendererAPI::DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t indexCount)
	{
	}

	void VulkanRendererAPI::DrawArray(const Ref<VertexArray>& vertexArray, uint32_t vertexCount) {}
}
