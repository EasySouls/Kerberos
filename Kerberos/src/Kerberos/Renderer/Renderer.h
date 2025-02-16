#pragma once

#include "RenderCommand.h"

namespace Kerberos
{
	class Renderer
	{
	public:
		static void BeginScene();
		static void EndScene();

		/// <summary>
		/// Binds the vertex array and submits it to the renderer
		/// </summary>
		/// <param name="vertexArray">The vertex array to be drawed with</param>
		static void Submit(const std::shared_ptr<VertexArray>& vertexArray);

		static RendererAPI::API GetAPI() { return RendererAPI::GetAPI(); }	
	};
}