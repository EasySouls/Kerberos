#pragma once
#include "Kerberos/Renderer/TextureCube.h"

#include <glad/glad.h>

namespace Kerberos
{
	class OpenGLTextureCube : public TextureCube
	{
	public:
		OpenGLTextureCube(std::string name, const std::vector<std::string>& faces,
			bool generateMipmaps, bool srgb);
		~OpenGLTextureCube() override;

		void Bind(uint32_t slot = 0) const override;

		uint32_t GetRendererID() const override { return m_RendererID; }
		const std::string& GetName() const override { return m_Name; }
		uint32_t GetWidth() const override;
		uint32_t GetHeight() const override;

		void SetData(void* data, uint32_t size) override;

		bool operator==(const Texture& other) const override
		{
			return m_RendererID == dynamic_cast<const OpenGLTextureCube&>(other).m_RendererID;
		}
	private:
		uint32_t m_RendererID;
		std::string m_Name;
		bool m_GenerateMipmaps;
		bool m_SRGB;
	};
}
