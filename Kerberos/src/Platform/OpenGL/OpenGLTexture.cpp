#include "kbrpch.h"
#include "OpenGLTexture.h"

#include "stb_image.h"
#include <glad/glad.h>

namespace Kerberos
{
	OpenGLTexture2D::OpenGLTexture2D(const std::string& path)
		: m_Path(path)
	{
		int width, height, channels;

		stbi_set_flip_vertically_on_load(true);
		stbi_uc* data = stbi_load(path.c_str(), &width, &height, &channels, 0);
	
		KBR_ASSERT(data, "Failed to load image!");

		m_Width = static_cast<unsigned int>(width);
		m_Height = static_cast<unsigned int>(height);

		glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID);
		glTextureStorage2D(m_RendererID, 1, GL_RGB8, static_cast<int>(m_Width), static_cast<int>(m_Height));

		glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		glTextureSubImage2D(m_RendererID, 
			0, 
			0, 
			0, 
			static_cast<int>(m_Width), 
			static_cast<int>(m_Width), 
			GL_RGB, 
			GL_UNSIGNED_BYTE, 
			data);

		stbi_image_free(data);
	}

	OpenGLTexture2D::~OpenGLTexture2D() 
	{
		glDeleteTextures(1, &m_RendererID);
	}

	void OpenGLTexture2D::Bind(const uint32_t slot) const 
	{
		glBindTextureUnit(slot, m_RendererID);
	}
}
