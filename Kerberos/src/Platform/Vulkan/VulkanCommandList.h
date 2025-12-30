#pragma once

#include "Kerberos/Renderer/CommandList.h"

#include <vulkan/vulkan.h>

namespace Kerberos
{
	class VulkanCommandList : public CommandList
	{
	public:
		VulkanCommandList();
		~VulkanCommandList() override = default;

		VulkanCommandList(const VulkanCommandList& other) = default;
		VulkanCommandList(VulkanCommandList&& other) noexcept = default;
		VulkanCommandList& operator=(const VulkanCommandList& other) = default;
		VulkanCommandList& operator=(VulkanCommandList&& other) noexcept = default;

		void Begin() override;
		void End() override;

		void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) override;
		void SetScissorRect(uint32_t x, uint32_t y, uint32_t width, uint32_t height) override;

		void BindPipeline(const Ref<GraphicsPipeline>& pipeline) override;
		void BindVertexBuffer(const Ref<VertexBuffer>& vertexBuffer, uint32_t slot) override;
		void BindIndexBuffer(const Ref<IndexBuffer>& indexBuffer) override;

		void DrawIndexed(uint32_t indexCount, uint32_t startIndexLocation = 0, int32_t baseVertexLocation = 0) override;
		void DrawVertexCount(uint32_t vertexCount, uint32_t startVertexLocation) override;

	private:
		VkCommandBuffer m_CommandBuffer = VK_NULL_HANDLE;
	};
}