#pragma once

#include "Kerberos/Renderer/RendererAPI.h"

#include <vulkan/vulkan.h>

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

	private:
		VkDevice m_Device = VK_NULL_HANDLE;
		VkCommandBuffer m_CommandBuffer = VK_NULL_HANDLE;
		VkImage m_Image = VK_NULL_HANDLE;
		VkExtent2D m_SwapChainExtent = {};
		VkFormat m_SwapChainImageFormat = {};
		VkRenderPass m_RenderPass = VK_NULL_HANDLE;
		VkPipelineLayout m_PipelineLayout = VK_NULL_HANDLE;
		VkPipeline m_GraphicsPipeline = VK_NULL_HANDLE;
	};
}
