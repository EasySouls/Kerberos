#include "kbrpch.h"
#include "Renderer.h"

namespace Kerberos
{
	void Renderer::BeginScene() 
	{
		// Transfer all the uniforms to the shaders
	}

	void Renderer::EndScene() 
	{
	
	}

	void Renderer::Submit(const std::shared_ptr<VertexArray>& vertexArray) 
	{
		vertexArray->Bind();
		RenderCommand::DrawIndexed(vertexArray);
	}
}
