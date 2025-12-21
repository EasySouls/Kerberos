#pragma once

#include "Kerberos/Core.h"

namespace Kerberos
{
	class CommandList
	{
	public:
		virtual ~CommandList() = default;

		virtual void Begin() = 0;
		virtual void End() = 0;
		/*virtual void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) = 0;
		virtual void SetScissorRect(uint32_t x, uint32_t y, uint32_t width, uint32_t height) = 0;
		virtual void ClearColor(float r, float g, float b, float a) = 0;
		virtual void ClearDepthStencil(float depth, uint8_t stencil) = 0;
		virtual void DrawIndexed(uint32_t indexCount, uint32_t startIndexLocation = 0, int32_t baseVertexLocation = 0) = 0;*/
	};
}