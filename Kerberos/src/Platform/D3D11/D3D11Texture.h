#pragma once

#include "Kerberos/Renderer/Texture.h"

#include <d3d11.h>
#include <dxgiformat.h>
#include <wrl.h>

namespace Kerberos
{
	using Microsoft::WRL::ComPtr;

	class D3D11Texture2D final : public Texture2D
	{
	using RendererID = uint32_t;

	public:
		explicit D3D11Texture2D(const std::string& path);
		explicit D3D11Texture2D(uint32_t width, uint32_t height);
		~D3D11Texture2D() override;

		uint32_t GetWidth() const override { return m_Width; }
		uint32_t GetHeight() const override { return m_Height; }

		uint32_t GetRendererID() const override { return m_RendererID; }

		void Bind(uint32_t slot = 0) const override;

		void SetData(void* data, uint32_t size) override;

		bool operator==(const Texture& other) const override
		{
			return m_RendererID == dynamic_cast<const D3D11Texture2D&>(other).m_RendererID;
		}

	private:
		std::string m_Path;
		uint32_t m_Width;
		uint32_t m_Height;
		RendererID m_RendererID;

		//! D3D11 specific members
		ComPtr<ID3D11Texture2D> m_Texture;
		ComPtr<ID3D11ShaderResourceView> m_ShaderResourceView;
		ComPtr<ID3D11SamplerState> m_SamplerState;
		DXGI_FORMAT m_Format;
	};
}

