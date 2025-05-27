#include "kbrpch.h"
#include "D3D11Texture.h"

#include "stb_image.h"

namespace Kerberos
{
	D3D11Texture2D::D3D11Texture2D(const std::string& path)
		: m_Path(path)
	{
		KBR_PROFILE_FUNCTION();

		int width, height, channels;

		stbi_set_flip_vertically_on_load(true);

		stbi_uc* data = nullptr;
		{
			KBR_PROFILE_SCOPE("stbi_load - OpenGLTexture2D::OpenGLTexture2D(const std::string& path)");
			data = stbi_load(path.c_str(), &width, &height, &channels, 0);
		}

		KBR_ASSERT(data, "Failed to load image!")

		m_Width = static_cast<uint32_t>(width);
		m_Height = static_cast<uint32_t>(height);


	}

	D3D11Texture2D::D3D11Texture2D(const uint32_t width, const uint32_t height)
		: m_Width(width), m_Height(height), m_RendererID(0), m_InternalFormat(DXGI_FORMAT_UNKNOWN), m_DataFormat(DXGI_FORMAT_UNKNOWN)
	{
		KBR_PROFILE_FUNCTION();
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
		m_Width = 0;
		m_Height = 0;
		m_InternalFormat = DXGI_FORMAT_UNKNOWN;
		m_DataFormat = DXGI_FORMAT_UNKNOWN;
	}

	void D3D11Texture2D::Bind(uint32_t slot) const
	{
		KBR_PROFILE_FUNCTION();
	}

	void D3D11Texture2D::SetData(void* data, uint32_t size)
	{
		KBR_PROFILE_FUNCTION();
	}
}
