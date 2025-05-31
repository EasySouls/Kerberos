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

		auto SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) -> void override;

		void SetClearColor(const glm::vec4& color) override;
		void Clear() override;

		void SetDepthTest(bool enabled) override;
		void SetDepthFunc(DepthFunc func) override;

		void DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t indexCount = 0) override;
		void DrawArray(const Ref<VertexArray>& vertexArray, uint32_t vertexCount) override;

	private:
		void CreateRenderPass();
		void CreateGraphicsPipeline();
		void CreateFramebuffers();
		void CreateCommandPool();
		void CreateCommandBuffer();

		void RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex) const;

	private:
		VkImage m_Image = VK_NULL_HANDLE;
		VkRenderPass m_RenderPass = VK_NULL_HANDLE;

		VkPipelineLayout m_PipelineLayout = VK_NULL_HANDLE;
		VkPipeline m_GraphicsPipeline = VK_NULL_HANDLE;
		std::vector<VkFramebuffer> m_SwapChainFramebuffers;
		VkCommandPool m_CommandPool = VK_NULL_HANDLE;
		VkCommandBuffer m_CommandBuffer = VK_NULL_HANDLE;
	};
}
