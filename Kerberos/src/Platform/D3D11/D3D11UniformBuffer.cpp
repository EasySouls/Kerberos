#include "kbrpch.h"
#include "D3D11UniformBuffer.h"

#include "D3D11Context.h"
#include "D3D11Utils.h"

namespace Kerberos
{
	
	D3D11UniformBuffer::D3D11UniformBuffer(uint32_t size, uint32_t binding)
	{
		D3D11_BUFFER_DESC desc{};
		desc.Usage = D3D11_USAGE::D3D11_USAGE_DYNAMIC;
		desc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_CONSTANT_BUFFER;
		desc.ByteWidth = size;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

		const auto& device = D3D11Context::Get().GetDevice();

		device->CreateBuffer(&desc, nullptr, m_Buffer.GetAddressOf());
	}

	void D3D11UniformBuffer::SetData(const void* data, const uint32_t size, uint32_t offset)
	{
		const auto& context = D3D11Context::Get().GetImmediateContext();

		D3D11_MAPPED_SUBRESOURCE mappedResource;
		const HRESULT mapRes = context->Map(m_Buffer.Get(), 0, D3D11_MAP::D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		KBR_CORE_ASSERT(SUCCEEDED(mapRes), "Failed to map uniform buffer!");

		memcpy(mappedResource.pData, data, size);
		context->Unmap(m_Buffer.Get(), 0);
	}

	void D3D11UniformBuffer::SetDebugName(const std::string& debugName)
	{
		const auto& context = D3D11Context::Get().GetImmediateContext();

		D3D11Utils::SetDebugName(context.Get(), debugName);
	}

}
