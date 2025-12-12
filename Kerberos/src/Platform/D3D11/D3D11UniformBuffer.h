#pragma once

#include "Kerberos/Renderer/UniformBuffer.h"

#include <d3d11_2.h>
#include <wrl/client.h>

using Microsoft::WRL::ComPtr;

namespace Kerberos
{
	class D3D11UniformBuffer : public UniformBuffer
	{
	public:
		explicit D3D11UniformBuffer(uint32_t size, uint32_t binding);

		void SetData(const void* data, uint32_t size, uint32_t offset) override;
		void SetDebugName(const std::string& debugName) override;

	private:
		ComPtr<ID3D11Buffer> m_Buffer = nullptr;
	};
}
