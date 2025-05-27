#include "kbrpch.h"
#include "D3D11RendererAPI.h"
#include "D3D11Context.h"

#include <glm/gtc/type_ptr.inl>


namespace Kerberos
{
	void D3D11RendererAPI::Init()
	{
	
	}

	void D3D11RendererAPI::SetViewport(const uint32_t x, const uint32_t y, const uint32_t width, const uint32_t height)
	{
		D3D11Context::Get().OnWindowResize(width, height);
	}

	void D3D11RendererAPI::SetClearColor(const glm::vec4& color)
	{
		// TODO: Set the clear color for the render target view
	}

	void D3D11RendererAPI::Clear()
	{
		//constexpr float clearDepth = 1.0f; // The value to clear the depth buffer to
		//constexpr UINT8 clearStencil = 0; // The value to clear the stencil buffer to

		//m_Context->ClearDepthStencilView(m_DepthStencilView.get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, clearDepth, clearStencil);
	}

	void D3D11RendererAPI::DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t indexCount)
	{
		
	}
}
