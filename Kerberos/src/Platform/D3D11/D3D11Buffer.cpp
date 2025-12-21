#include "kbrpch.h"
#include "D3D11Buffer.h"

#include "D3D11Context.h"
#include "D3D11Utils.h"

namespace Kerberos
{
	////////////////////// Vertex Buffer //////////////////////

	D3D11VertexBuffer::D3D11VertexBuffer(const float* vertices, const uint32_t size)
	{
		D3D11_BUFFER_DESC bufferDesc{};
		bufferDesc.Usage = D3D11_USAGE_DEFAULT;
		bufferDesc.ByteWidth = size;
		bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bufferDesc.CPUAccessFlags = 0;
		bufferDesc.MiscFlags = 0;

		D3D11_SUBRESOURCE_DATA initData{};
		initData.pSysMem = vertices;
		initData.SysMemPitch = 0;
		initData.SysMemSlicePitch = 0;

		const auto& device = D3D11Context::Get().GetDevice();

		const HRESULT hr = device->CreateBuffer(&bufferDesc, &initData, m_Buffer.GetAddressOf());
		KBR_CORE_ASSERT(SUCCEEDED(hr), "Failed to create D3D11 vertex buffer!");
	}

	D3D11VertexBuffer::D3D11VertexBuffer(const uint32_t size)
	{
		D3D11_BUFFER_DESC bufferDesc{};
		bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		bufferDesc.ByteWidth = size;
		bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		bufferDesc.MiscFlags = 0;

		const auto& device = D3D11Context::Get().GetDevice();
		const HRESULT hr = device->CreateBuffer(&bufferDesc, nullptr, m_Buffer.GetAddressOf());
		KBR_CORE_ASSERT(SUCCEEDED(hr), "Failed to create D3D11 dynamic vertex buffer!");
	}

	D3D11VertexBuffer::~D3D11VertexBuffer()
	{
		if (m_Buffer)
		{
			m_Buffer.Reset();
		}
	}

	void D3D11VertexBuffer::SetData(const void* data, const uint32_t size)
	{
		const auto& context = D3D11Context::Get().GetImmediateContext();

		D3D11_MAPPED_SUBRESOURCE mappedResource;
		const HRESULT mapRes = context->Map(m_Buffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		KBR_CORE_ASSERT(SUCCEEDED(mapRes), "Failed to map vertex buffer!");

		std::memcpy(mappedResource.pData, data, size);
		context->Unmap(m_Buffer.Get(), 0);
	}

	void D3D11VertexBuffer::Bind() const
	{
		const auto& context = D3D11Context::Get().GetImmediateContext();

		const uint32_t stride = /*sizeof(float) * */m_Layout.GetStride();

		context->IASetVertexBuffers(
			0,
			1,
			m_Buffer.GetAddressOf(),
			&stride,
			nullptr);
	}

	void D3D11VertexBuffer::Unbind() const
	{
	}

	void D3D11VertexBuffer::SetDebugName(const std::string& name) 
	{
		D3D11Utils::SetDebugName(m_Buffer.Get(), name);
	}

	////////////////////// Index Buffer //////////////////////

	D3D11IndexBuffer::D3D11IndexBuffer(const uint32_t* indices, const uint32_t count)
		: m_Count(count)
	{
		D3D11_BUFFER_DESC bufferDesc{};
		bufferDesc.Usage = D3D11_USAGE_DEFAULT;
		bufferDesc.ByteWidth = sizeof(uint32_t) * count;
		bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		bufferDesc.CPUAccessFlags = 0;
		bufferDesc.MiscFlags = 0;

		D3D11_SUBRESOURCE_DATA initData;
		initData.pSysMem = indices;
		initData.SysMemPitch = 0;
		initData.SysMemSlicePitch = 0;
		const auto& device = D3D11Context::Get().GetDevice();
		const HRESULT hr = device->CreateBuffer(&bufferDesc, &initData, m_Buffer.GetAddressOf());

		KBR_CORE_ASSERT(SUCCEEDED(hr), "Failed to create D3D11 index buffer!");
	}

	D3D11IndexBuffer::~D3D11IndexBuffer()
	{
		if (m_Buffer)
		{
			m_Buffer.Reset();
		}
	}

	void D3D11IndexBuffer::Bind() const
	{
		const auto& context = D3D11Context::Get().GetImmediateContext();

		context->IASetIndexBuffer(m_Buffer.Get(), DXGI_FORMAT_R32_UINT, 0);
	}

	void D3D11IndexBuffer::Unbind() const
	{
	}

	void D3D11IndexBuffer::SetDebugName(const std::string& name) 
	{
		D3D11Utils::SetDebugName(m_Buffer.Get(), name);
	}
}
