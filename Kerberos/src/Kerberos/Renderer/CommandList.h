#pragma once

#include "Kerberos/Core.h"

namespace Kerberos
{
	class IndexBuffer;
	class VertexBuffer;
	class GraphicsPipeline;

	class CommandList
	{
	public:
		virtual ~CommandList() = default;

		virtual void Begin() = 0;
		virtual void End() = 0;

		virtual void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) = 0;
		virtual void SetScissorRect(uint32_t x, uint32_t y, uint32_t width, uint32_t height) = 0;

		virtual void BindPipeline(const Ref<GraphicsPipeline>& pipeline) = 0;
		//virtual void BindPipeline(const Ref<ComputePipeline>& pipeline) = 0;
		virtual void BindVertexBuffer(const Ref<VertexBuffer>& vertexBuffer, uint32_t slot = 0) = 0;
		virtual void BindIndexBuffer(const Ref<IndexBuffer>& indexBuffer) = 0;

		virtual void DrawIndexed(uint32_t indexCount, uint32_t startIndexLocation = 0, int32_t baseVertexLocation = 0) = 0;
		virtual void DrawVertexCount(uint32_t vertexCount, uint32_t startVertexLocation = 0) = 0;

		/*virtual void ClearColor(float r, float g, float b, float a) = 0;
		virtual void ClearDepthStencil(float depth, uint8_t stencil) = 0;*/

		static Ref<CommandList> Create();
	};
}