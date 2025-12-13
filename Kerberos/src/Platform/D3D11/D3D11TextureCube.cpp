#include "kbrpch.h"
#include "D3D11TextureCube.h"

#include "D3D11Context.h"
#include "Kerberos/Core/Exceptions.h"
#include "Utils/TextureUtils.h"

namespace Kerberos
{
	D3D11TextureCube::D3D11TextureCube(CubemapData data) 
		: m_Data(std::move(data))
	{
		KBR_CORE_ASSERT(m_Data.Faces.size() == 6, "D3D11TextureCube must have 6 faces.");

		const uint32_t width = m_Data.Faces[0].Specification.Width;
		const uint32_t height = m_Data.Faces[0].Specification.Height;

		D3D11_TEXTURE2D_DESC texDesc = {};
		texDesc.Width = width;
		texDesc.Height = height;
		texDesc.MipLevels = 1;
		texDesc.ArraySize = 6;
		texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // TODO: Determine format based on m_Data
		texDesc.SampleDesc.Count = 1;
		texDesc.SampleDesc.Quality = 0;
		texDesc.CPUAccessFlags = 0;
		texDesc.Usage = D3D11_USAGE_DEFAULT;
		texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		texDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;

		const auto& device = D3D11Context::Get().GetDevice();
		const auto& context = D3D11Context::Get().GetImmediateContext();

		constexpr uint32_t bytesPerPixel = 4;
		const uint32_t expectedSize = width * height * bytesPerPixel;

		D3D11_SUBRESOURCE_DATA subresourceData[6] = {};

		for (size_t i = 0; i < 6; ++i)
		{
			KBR_CORE_ASSERT(m_Data.Faces[i].Buffer.Data, "Texture data is null!");

			if (m_Data.Faces[i].Buffer.Size != expectedSize)
			{
				KBR_CORE_ERROR("Face {0} data size ({1}) does not match expected size ({2}).", i, m_Data.Faces[i].Buffer.Size, expectedSize);
			}

			subresourceData[i].pSysMem = m_Data.Faces[i].Buffer.Data;
			subresourceData[i].SysMemPitch = width * 4;
			subresourceData[i].SysMemSlicePitch = 0;
		}

		HRESULT hr = device->CreateTexture2D(&texDesc, subresourceData, m_CubeTexture.GetAddressOf());
		KBR_CORE_ASSERT(SUCCEEDED(hr), "Failed to create D3D11 texture cube.");

		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format = texDesc.Format;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
		srvDesc.TextureCube.MipLevels = texDesc.MipLevels;
		srvDesc.TextureCube.MostDetailedMip = 0;

		hr = device->CreateShaderResourceView(m_CubeTexture.Get(), &srvDesc, m_SRV.GetAddressOf());
		KBR_CORE_ASSERT(SUCCEEDED(hr), "Failed to create shader resource view for D3D11 texture cube.");

		m_RendererID = reinterpret_cast<RendererID>(m_SRV.Get());
	}

	D3D11TextureCube::~D3D11TextureCube() = default;

	void D3D11TextureCube::Bind(const uint32_t slot) const
	{
		const auto& context = D3D11Context::Get().GetImmediateContext();
		context->PSSetShaderResources(slot, 1, m_SRV.GetAddressOf());
	}

	uint32_t D3D11TextureCube::GetWidth() const
	{
		return m_Data.Faces[0].Specification.Width;
	}

	uint32_t D3D11TextureCube::GetHeight() const
	{
		return m_Data.Faces[0].Specification.Height;
	}

	[[noreturn]]
	void D3D11TextureCube::SetData(void* data, uint32_t size)
	{
		/// Might have to change signature, depending on how we want to set data for each face
		throw NotImplementedException("D3D11TextureCube::SetData is not implemented.");
	}

	void D3D11TextureCube::SetDebugName(const std::string& name) const 
	{
		const std::string textureName = name + " Texture";
		m_CubeTexture->SetPrivateData(WKPDID_D3DDebugObjectName, static_cast<uint32_t>(name.size()), textureName.c_str());

		const std::string srvName = name + " Shader Resource View";
		m_SRV->SetPrivateData(WKPDID_D3DDebugObjectName, static_cast<uint32_t>(name.size()), name.c_str());
	}
}
