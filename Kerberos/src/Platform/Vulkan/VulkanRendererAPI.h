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

		auto SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) -> void override;

		void SetClearColor(const glm::vec4& color) override;
		void Clear() override;
		void ClearDepth() override;

		void SetDepthTest(bool enabled) override;
		void SetDepthFunc(DepthFunc func) override;

		void DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t indexCount = 0) override;
		void DrawArray(const Ref<VertexArray>& vertexArray, uint32_t vertexCount) override;

	private:
		glm::vec4 m_ClearColor = { 0.0f, 0.0f, 0.0f, 1.0f };
	};
}
