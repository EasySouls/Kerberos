#pragma once

#include "Kerberos/Renderer/RendererAPI.h"

#include <vulkan/vulkan.h>

namespace Kerberos
{
	class VulkanRendererAPI : public RendererAPI
	{
	public:
		void Init() override;
		void SetupPipeline();

		void SetViewport(const uint32_t x, const uint32_t y, const uint32_t width, const uint32_t height) override;

		void SetClearColor(const glm::vec4& color) override;
		void Clear() override;

		void DrawIndexed(const Ref<VertexArray>& vertexArray) override;

	private:
		VkCommandBuffer m_CommandBuffer = VK_NULL_HANDLE;
		VkImage m_Image = VK_NULL_HANDLE;
	};
}