#include "kbrpch.h"
#include "D3D11UniformBuffer.h"

#include "D3D11Context.h"
#include "D3D11Utils.h"

namespace Kerberos
{
	
	D3D11UniformBuffer::D3D11UniformBuffer(const uint32_t size, const uint32_t binding)
		: m_Binding(binding), m_Size(size)
	{
		D3D11_BUFFER_DESC desc{};
		desc.Usage = D3D11_USAGE_DYNAMIC;
		desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		desc.ByteWidth = m_Size;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

		const auto& device = D3D11Context::Get().GetDevice();

		if (FAILED(device->CreateBuffer(&desc, nullptr, m_Buffer.GetAddressOf())))
		{
			KBR_CORE_ASSERT(false, "Failed to create D3D11 constant buffer!");
		}
	}

	void D3D11UniformBuffer::SetData(const void* data, const uint32_t size, const uint32_t offset)
	{
		KBR_CORE_ASSERT(offset + size <= m_Size, "Data size exceeds constant buffer size!");

		const auto& context = D3D11Context::Get().GetImmediateContext();

		D3D11_MAPPED_SUBRESOURCE mappedResource;
		const HRESULT mapRes = context->Map(m_Buffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		KBR_CORE_ASSERT(SUCCEEDED(mapRes), "Failed to map uniform buffer!");

		std::memcpy(static_cast<uint8_t*>(mappedResource.pData) + offset, data, size);
		context->Unmap(m_Buffer.Get(), 0);
	}

	void D3D11UniformBuffer::Bind()
	{
		const auto& context = D3D11Context::Get().GetImmediateContext();
		context->VSSetConstantBuffers(m_Binding, 1, m_Buffer.GetAddressOf());
		context->PSSetConstantBuffers(m_Binding, 1, m_Buffer.GetAddressOf());
	}

	void D3D11UniformBuffer::SetDebugName(const std::string& debugName)
	{
		D3D11Utils::SetDebugName(m_Buffer.Get(), debugName);
	}

}
