#pragma once

#include "Kerberos/Renderer/Texture.h"

namespace Kerberos
{
	class VulkanTexture2D final : public Texture2D
	{
		using RendererID = uint64_t;

	public:
		explicit VulkanTexture2D(const std::string& path);
		explicit VulkanTexture2D(uint32_t width, uint32_t height);
		~VulkanTexture2D() override;

		uint32_t GetWidth() const override { return m_Width; }
		uint32_t GetHeight() const override { return m_Height; }

		RendererID GetRendererID() const override { return m_RendererID; }

		void Bind(uint32_t slot = 0) const override;

		void SetData(void* data, uint32_t size) override;

		bool operator==(const Texture& other) const override
		{
			return m_RendererID == dynamic_cast<const VulkanTexture2D&>(other).m_RendererID;
		}

	private:
		RendererID m_RendererID = 0;
		uint32_t m_Width = 0;
		uint32_t m_Height = 0;
	};
}

