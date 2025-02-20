#pragma once

#include "RendererAPI.h"

namespace Kerberos
{
	class RenderCommand
	{
	public:
		inline static void Init() { s_RendererAPI->Init(); }

		inline static void SetClearColor(const glm::vec4& color) { s_RendererAPI->SetClearColor(color); }
		inline static void Clear() { s_RendererAPI->Clear(); }

		inline static void DrawIndexed(const std::shared_ptr<VertexArray>& vertexArray) { s_RendererAPI->DrawIndexed(vertexArray); }

		//static void SetupRendererAPI() { s_RendererAPI = RendererAPI::Create(); }
		static void SetupRendererAPI();

	private:
		static RendererAPI* s_RendererAPI;
	};
}