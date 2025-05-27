#include "kbrpch.h"
#include "D3D11Buffer.h"

namespace Kerberos
{

	D3D11VertexBuffer::D3D11VertexBuffer(const float* vertices, uint32_t size)
	{}

	D3D11VertexBuffer::D3D11VertexBuffer(uint32_t size)
	{}

	D3D11VertexBuffer::~D3D11VertexBuffer()
	{}

	void D3D11VertexBuffer::SetData(const void* data, uint32_t size)
	{}

	void D3D11VertexBuffer::Bind() const
	{}

	void D3D11VertexBuffer::Unbind() const
	{}

	D3D11IndexBuffer::D3D11IndexBuffer(const uint32_t* indices, uint32_t count)
	{
	}

	D3D11IndexBuffer::~D3D11IndexBuffer()
	{
	}

	void D3D11IndexBuffer::Bind() const
	{
	}

	void D3D11IndexBuffer::Unbind() const
	{
	}
}
