#pragma once

#include "RendererAPI.h"

namespace Kerberos
{
	class RenderCommand
	{
	public:
		static void Init() { s_RendererAPI->Init(); }

		static void SetViewport(const uint32_t x, const uint32_t y, const uint32_t width, const uint32_t height) 
		{ 
			s_RendererAPI->SetViewport(x, y, width, height); 
		}

		static void SetClearColor(const glm::vec4& color) { s_RendererAPI->SetClearColor(color); }
		static void Clear() { s_RendererAPI->Clear(); }
		static void ClearDepth() { s_RendererAPI->ClearDepth(); }

		static void SetDepthTest(const bool enabled) { s_RendererAPI->SetDepthTest(enabled); }
		static void SetDepthFunc(const DepthFunc func) { s_RendererAPI->SetDepthFunc(func); }

		static void DrawIndexed(const Ref<VertexArray>& vertexArray, const uint32_t indexCount = 0) 
		{
			s_RendererAPI->DrawIndexed(vertexArray, indexCount); 
		}

		static void DrawArray(const Ref<VertexArray>& vertexArray, const uint32_t vertexCount)
		{
			s_RendererAPI->DrawArray(vertexArray, vertexCount);
		}

		// void SetupRendererAPI() { s_RendererAPI = RendererAPI::Create(); }
		static void SetupRendererAPI();

	private:
		static RendererAPI* s_RendererAPI;
	};
}