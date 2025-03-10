#pragma once

#include "Kerberos/Renderer/RendererAPI.h"

#include <vulkan/vulkan.h>

#include "VulkanContext.h"

namespace Kerberos
{
	class VulkanRendererAPI : public RendererAPI
	{
	public:
		void Init() override;
		void Cleanup() const;

		void SetViewport(const uint32_t x, const uint32_t y, const uint32_t width, const uint32_t height) override;

		void SetClearColor(const glm::vec4& color) override;
		void Clear() override;

		void DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t indexCount = 0) override;

	private:
		void CreateRenderPass();
		void CreateGraphicsPipeline();
		void CreateFramebuffers();
		void CreateCommandPool();
		void CreateCommandBuffer();

		void RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);

	private:
		Scope<VulkanContext> m_Context;

		VkDevice m_Device = VK_NULL_HANDLE;
		VkImage m_Image = VK_NULL_HANDLE;
		VkExtent2D m_SwapChainExtent = {};
		VkFormat m_SwapChainImageFormat = {};
		VkRenderPass m_RenderPass = VK_NULL_HANDLE;
		std::vector<VkImageView> m_SwapChainImageViews;

		VkPipelineLayout m_PipelineLayout = VK_NULL_HANDLE;
		VkPipeline m_GraphicsPipeline = VK_NULL_HANDLE;
		std::vector<VkFramebuffer> m_SwapChainFramebuffers;
		VkCommandPool m_CommandPool = VK_NULL_HANDLE;
		VkCommandBuffer m_CommandBuffer = VK_NULL_HANDLE;
	};
}
