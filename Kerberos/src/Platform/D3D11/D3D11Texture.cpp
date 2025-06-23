#include "kbrpch.h"
#include "D3D11Texture.h"

#include "stb_image.h"
#include <d3d11.h>

#include "D3D11Context.h"

namespace Kerberos
{
	namespace Utils
	{
		uint32_t GetBytesPerPixel(const DXGI_FORMAT fmt)
		{
			switch (fmt)
			{
			case DXGI_FORMAT_R32G32B32A32_FLOAT: return 16;
			case DXGI_FORMAT_R32G32B32A32_UINT:
			case DXGI_FORMAT_R32G32B32A32_SINT: return 16;

			case DXGI_FORMAT_R16G16B16A16_FLOAT:
			case DXGI_FORMAT_R16G16B16A16_UNORM:
			case DXGI_FORMAT_R16G16B16A16_UINT:
			case DXGI_FORMAT_R16G16B16A16_SNORM:
			case DXGI_FORMAT_R16G16B16A16_SINT: return 8;

			case DXGI_FORMAT_R8G8B8A8_UNORM:
			case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
			case DXGI_FORMAT_R8G8B8A8_UINT:
			case DXGI_FORMAT_R8G8B8A8_SNORM:
			case DXGI_FORMAT_R8G8B8A8_SINT:
			case DXGI_FORMAT_B8G8R8A8_UNORM:
			case DXGI_FORMAT_B8G8R8X8_UNORM:
			case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB: return 4;

			case DXGI_FORMAT_R32_FLOAT:
			case DXGI_FORMAT_R32_UINT:
			case DXGI_FORMAT_R32_SINT: return 4;

			default:
				KBR_ASSERT(false, "Unsupported DXGI_FORMAT in GetBytesPerPixel");
				return 0;
			}
		}
	
	}
	D3D11Texture2D::D3D11Texture2D(const std::string& path)
		: m_Path(path)
	{
		KBR_PROFILE_FUNCTION();

		int width, height, channels;

		stbi_set_flip_vertically_on_load(true);

		stbi_uc* imageData = nullptr;
		{
			KBR_PROFILE_SCOPE("stbi_load - OpenGLTexture2D::OpenGLTexture2D(const std::string& path)");
			/// Since DirectX doesn't have a specific RGB format, we load images as RGBA
			imageData = stbi_load(path.c_str(), &width, &height, &channels, 4);
		}

		KBR_ASSERT(imageData, "Failed to load image!")

		m_Spec.Width = static_cast<uint32_t>(width);
		m_Spec.Height = static_cast<uint32_t>(height);

		m_Format = DXGI_FORMAT_UNKNOWN;
		if (channels == 4) /// RGBA
		{
			m_Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		}
		else if (channels == 3) /// RGB
		{
			/// Note: Direct3D does not have a specific RGB format, so we use RGBA with no alpha
			m_Format = DXGI_FORMAT_R8G8B8A8_UINT;
		}

		D3D11_TEXTURE2D_DESC desc;
		ZeroMemory(&desc, sizeof(desc));
		desc.Width = m_Spec.Width;
		desc.Height = m_Spec.Height;
		desc.MipLevels = 1;
		desc.ArraySize = 1;
		desc.Format = m_Format;
		desc.SampleDesc.Count = 1;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		desc.CPUAccessFlags = 0;

		D3D11_SUBRESOURCE_DATA resData;
		resData.pSysMem = imageData;
		resData.SysMemPitch = m_Spec.Width * (channels == 4 ? 4 : 3);
		resData.SysMemSlicePitch = 0;

		const auto device = D3D11Context::Get().GetDevice();
		HRESULT hr = device->CreateTexture2D(&desc, &resData, m_Texture.GetAddressOf());
		if (FAILED(hr))
		{
			stbi_image_free(imageData);
			KBR_ERROR("Failed to create texture from image: {0}", path);
			return;
		}

		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
		ZeroMemory(&srvDesc, sizeof(srvDesc));
		srvDesc.Format = m_Format;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = desc.MipLevels;
		srvDesc.Texture2D.MostDetailedMip = 0;

		hr = device->CreateShaderResourceView(m_Texture.Get(), &srvDesc, m_ShaderResourceView.GetAddressOf());
		if (FAILED(hr))
		{
			stbi_image_free(imageData);
			KBR_ERROR("Failed to create shader resource view for texture: {0}", path);
			return;
		}

		m_RendererID = reinterpret_cast<RendererID>(m_ShaderResourceView.Get());

		stbi_image_free(imageData);
	}

	D3D11Texture2D::D3D11Texture2D(const TextureSpecification& spec)
		: m_Spec(spec)
	{
		KBR_PROFILE_FUNCTION();

		/// For now, we will use RGBA format as a default
		m_Format = DXGI_FORMAT_R8G8B8A8_UNORM; 

		D3D11_TEXTURE2D_DESC desc;
		ZeroMemory(&desc, sizeof(desc));
		desc.Width = m_Spec.Width;
		desc.Height = m_Spec.Height;
		desc.MipLevels = 1;
		desc.ArraySize = 1;
		desc.Format = m_Format;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Usage = D3D11_USAGE_DEFAULT; // Allows UpdateSubresource. For Map/Unmap, use D3D11_USAGE_DYNAMIC and D3D11_CPU_ACCESS_WRITE
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		desc.CPUAccessFlags = 0;
		desc.MiscFlags = 0;

		const auto device = D3D11Context::Get().GetDevice();

		/// The texture will be created without initial data, so we pass nullptr for the initial data
		HRESULT hr = device->CreateTexture2D(&desc, nullptr, m_Texture.GetAddressOf());
		if (FAILED(hr))
		{
			KBR_ERROR("Failed to create texture with dimensions {0}x{1}", m_Spec.Width, m_Spec.Height);
			return;
		}

		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
		ZeroMemory(&srvDesc, sizeof(srvDesc));
		srvDesc.Format = m_Format;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = desc.MipLevels;
		srvDesc.Texture2D.MostDetailedMip = 0;

		hr = device->CreateShaderResourceView(m_Texture.Get(), &srvDesc, m_ShaderResourceView.ReleaseAndGetAddressOf());
		if (FAILED(hr))
		{
			KBR_ERROR("Failed to create shader resource view for D3D11Texture2D. HRESULT: 0x%X", hr);
			m_Texture.Reset();
			return;
		}

		m_RendererID = reinterpret_cast<RendererID>(m_ShaderResourceView.Get());

	}

	D3D11Texture2D::~D3D11Texture2D()
	{
		KBR_PROFILE_FUNCTION();

		if (m_Texture)
		{
			m_Texture.Reset();
		}
		if (m_ShaderResourceView)
		{
			m_ShaderResourceView.Reset();
		}
		if (m_SamplerState)
		{
			m_SamplerState.Reset();
		}
		m_RendererID = 0;
		m_Spec.Width = 0;
		m_Spec.Height = 0;
	}

	void D3D11Texture2D::Bind(const uint32_t slot) const
	{
		KBR_PROFILE_FUNCTION();

		KBR_CORE_ASSERT(m_Texture, "Texture is not initialized!");
		KBR_CORE_ASSERT(m_ShaderResourceView, "Shader Resource View is not initialized!");
		KBR_CORE_ASSERT(slot < D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT, "Invalid shader resource slot!");

		const auto context = D3D11Context::Get().GetImmediateContext();
		context->PSSetShaderResources(slot, 1, m_ShaderResourceView.GetAddressOf());
		//context->PSSetSamplers(slot, 1, m_SamplerState.GetAddressOf());
	}

	void D3D11Texture2D::SetData(void* data, const uint32_t size)
	{
		KBR_PROFILE_FUNCTION();

		KBR_CORE_ASSERT(data, "Data pointer cannot be null for SetData.");
		KBR_CORE_ASSERT(m_Texture, "Texture must be created before calling SetData.");

		if (!m_Texture)
		{
			KBR_ERROR("Texture not initialized. Cannot set data.");
			return;
		}

		const uint32_t bytesPerPixel = Utils::GetBytesPerPixel(m_Format);

		KBR_CORE_ASSERT(size == m_Spec.Width * m_Spec.Height * bytesPerPixel, "Data size does not match texture size.");

		const auto context = D3D11Context::Get().GetImmediateContext();
		const uint32_t rowPitch = m_Spec.Width * bytesPerPixel;
		context->UpdateSubresource(m_Texture.Get(), 0, nullptr, data, rowPitch, 0);
	}
}
