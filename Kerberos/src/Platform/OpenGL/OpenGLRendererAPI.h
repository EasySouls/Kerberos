#pragma once

#include "Kerberos/Renderer/RendererAPI.h"

namespace Kerberos
{
	class OpenGLRendererAPI : public RendererAPI
	{
	public:
		void Init() override;

		void SetViewport(const uint32_t x, const uint32_t y, const uint32_t width, const uint32_t height) override;

		void SetClearColor(const glm::vec4& color) override;
		void Clear() override;
		void ClearDepth() override;

		void SetDepthTest(bool enabled) override;
		void SetDepthFunc(DepthFunc func) override;

		void DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t indexCount = 0) override;
		void DrawArray(const Ref<VertexArray>& vertexArray, const uint32_t vertexCount) override;
	};
}

