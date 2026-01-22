#pragma once

#include "Kerberos/Renderer/CommandList.h"

namespace Kerberos
{
	class OpenGLCommandList : public CommandList
	{
	public:
		OpenGLCommandList() = default;
		~OpenGLCommandList() override = default;

		void Begin() override;
		void End() override;

		void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) override;
		void SetScissorRect(uint32_t x, uint32_t y, uint32_t width, uint32_t height) override;

		void BindPipeline(const Ref<GraphicsPipeline>& pipeline) override;
		void BindVertexBuffer(const Ref<VertexBuffer>& vertexBuffer, uint32_t slot) override;
		void BindIndexBuffer(const Ref<IndexBuffer>& indexBuffer) override;

		void DrawIndexed(uint32_t indexCount, uint32_t startIndexLocation, int32_t baseVertexLocation) override;
		void DrawVertexCount(uint32_t vertexCount, uint32_t startVertexLocation) override;
	};
}