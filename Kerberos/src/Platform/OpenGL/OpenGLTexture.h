#pragma once

#include "glad/glad.h"
#include "Kerberos/Renderer/Texture.h"

namespace Kerberos
{
	class OpenGLTexture2D : public Texture2D
	{
	public:
		explicit OpenGLTexture2D(const std::string& path);
		explicit OpenGLTexture2D(uint32_t width, uint32_t height);
		~OpenGLTexture2D() override;

		uint32_t GetWidth() const override { return m_Width; }
		uint32_t GetHeight() const override { return m_Height; }

		uint32_t GetRendererID() const override { return m_RendererID; }
		
		void Bind(uint32_t slot = 0) const override;

		void SetData(void* data, uint32_t size) override;

		bool operator==(const Texture& other) const override
		{
			return m_RendererID == dynamic_cast<const OpenGLTexture2D&>(other).m_RendererID;
		}

	private:
		std::string m_Path;
		uint32_t m_Width;
		uint32_t m_Height;
		GLenum m_InternalFormat;
		GLenum m_DataFormat;
		uint32_t m_RendererID;
	};
}
