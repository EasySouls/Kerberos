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

		static void DrawIndexed(const std::shared_ptr<VertexArray>& vertexArray) { s_RendererAPI->DrawIndexed(vertexArray); }

		//static void SetupRendererAPI() { s_RendererAPI = RendererAPI::Create(); }
		static void SetupRendererAPI();

	private:
		static RendererAPI* s_RendererAPI;
	};
}