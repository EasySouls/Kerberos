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
		m_ClearColor = color;
	}

	void D3D11RendererAPI::Clear()
	{
		const auto rtv = D3D11Context::Get().GetRenderTargetView();
		const auto context = D3D11Context::Get().GetDeviceContext();

		if (!rtv || !context)
		{
			KBR_CORE_ASSERT(false, "Render target view or device context is null!");
			return;
		}

		context->ClearRenderTargetView(rtv.Get(), glm::value_ptr(m_ClearColor));
	}

	void D3D11RendererAPI::DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t indexCount)
	{
		const auto context = D3D11Context::Get().GetDeviceContext();
		if (!context)
		{
			KBR_CORE_ASSERT(false, "Device context is null!");
			return;
		}
		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		/*context->IASetVertexBuffers(0, 1, vertexArray->GetVertexBuffer().GetAddressOf(), vertexArray->GetVertexBuffer()->GetStridePtr(), vertexArray->GetVertexBuffer()->GetOffsetPtr());
		context->IASetIndexBuffer(vertexArray->GetIndexBuffer()->GetBuffer().Get(), vertexArray->GetIndexBuffer()->GetFormat(), 0);
		context->DrawIndexed(indexCount ? indexCount : vertexArray->GetIndexBuffer()->GetCount(), 0, 0);*/
	}
}
