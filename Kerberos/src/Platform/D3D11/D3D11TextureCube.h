#pragma once

#include "Kerberos/Renderer/TextureCube.h"

#include <d3d11.h>

namespace Kerberos
{
	class D3D11TextureCube final : public TextureCube
	{
	public:
		using RendererID = uint64_t;

		explicit D3D11TextureCube(const CubemapData& data);
		~D3D11TextureCube() override;

		void Bind(uint32_t slot = 0) const override;

		uint64_t GetRendererID() const override { return m_RendererID; }
		const std::string& GetName() const override { return m_Name; }
		uint32_t GetWidth() const override;
		uint32_t GetHeight() const override;
		const TextureSpecification& GetSpecification() const override { return m_Spec; }

		void SetData(void* data, uint32_t size) override;

		bool operator==(const Texture& other) const override
		{
			return m_RendererID == dynamic_cast<const D3D11TextureCube&>(other).m_RendererID;
		}

		void SetDebugName(const std::string& name) const override;

	private:
		RendererID m_RendererID;
		std::string m_Name;
		bool m_GenerateMipmaps;
		bool m_SRGB;
		TextureSpecification m_Spec;
	};
}


