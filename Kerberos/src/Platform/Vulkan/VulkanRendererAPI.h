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

		void SetClearColor(const glm::vec4& color) override;
		void Clear() override;

		void DrawIndexed(const Ref<VertexArray>& vertexArray) override;

	private:
		VkCommandBuffer m_CommandBuffer = VK_NULL_HANDLE;
		VkImage m_Image = VK_NULL_HANDLE;
	};
}