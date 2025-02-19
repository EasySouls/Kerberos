#include "kbrpch.h"
#include "D3D11RendererAPI.h"

#include <glm/gtc/type_ptr.inl>


namespace Kerberos
{
	void D3D11RendererAPI::Init()
	{
	
	}

	void D3D11RendererAPI::SetClearColor(const glm::vec4& color)
	{
		/*const float clearColor[4] = { color.r, color.g, color.b, color.a };
		m_Context->ClearRenderTargetView(m_Target.get(), clearColor);*/
		m_Context->ClearRenderTargetView(m_Target.get(), glm::value_ptr(color));
	}

	void D3D11RendererAPI::Clear()
	{
		constexpr float clearDepth = 1.0f; // The value to clear the depth buffer to
		constexpr UINT8 clearStencil = 0; // The value to clear the stencil buffer to

		m_Context->ClearDepthStencilView(m_DepthStencilView.get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, clearDepth, clearStencil);
	}

	void D3D11RendererAPI::DrawIndexed(const Ref<VertexArray>& vertexArray)
	{
		
	}
}
