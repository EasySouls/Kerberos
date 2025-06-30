#include "kbrpch.h"
#include "D3D11TextureCube.h"

namespace Kerberos
{
	D3D11TextureCube::D3D11TextureCube(const CubemapData& data) 
	{
		throw std::runtime_error("D3D11TextureCube::D3D11TextureCube(const CubemapData&) is not yet implemented.");
	}

	D3D11TextureCube::~D3D11TextureCube() = default;

	void D3D11TextureCube::Bind(uint32_t slot) const
	{
	}

	uint32_t D3D11TextureCube::GetWidth() const
	{
		throw std::runtime_error("D3D11TextureCube::GetWidth() is not yet implemented.");
	}

	uint32_t D3D11TextureCube::GetHeight() const
	{
		throw std::runtime_error("D3D11TextureCube::GetWidth() is not yet implemented.");
	}

	void D3D11TextureCube::SetData(void* data, uint32_t size)
	{
		throw std::runtime_error("D3D11TextureCube::GetWidth() is not yet implemented.");
	}
}
