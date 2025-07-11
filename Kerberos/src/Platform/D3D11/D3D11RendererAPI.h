#pragma once

#include "Kerberos/Core.h"
#include "Kerberos/Renderer/RendererAPI.h"

#include <d3d11.h>

namespace Kerberos
{
	class D3D11RendererAPI final : public RendererAPI
	{
	public:
		void Init() override;

		void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) override;

		void SetClearColor(const glm::vec4& color) override;
		void Clear() override;
		void ClearDepth() override;

		void SetDepthTest(bool enabled) override {}
		void SetDepthFunc(DepthFunc func) override {}

		void DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t indexCount = 0) override;
		void DrawArray(const Ref<VertexArray>& vertexArray, const uint32_t vertexCount) override {}

	private:
		glm::vec4 m_ClearColor = glm::vec4(0.2f, 0.2f, 0.2f, 1.0f);
	};
}
