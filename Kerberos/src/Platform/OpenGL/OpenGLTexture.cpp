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

		/// Internal format is how OpenGl will store the texture data internally (in the GPU)
		GLenum internalFormat = 0;
		/// Data format is the format of the texture data we provide to OpenGL
		GLenum dataFormat = 0;
		
		if (channels == 4)
		{
			internalFormat = GL_RGBA8;
			dataFormat = GL_RGBA;
		}
		else if (channels == 3)
		{
			internalFormat = GL_RGB8;
			dataFormat = GL_RGB;
		}

		KBR_ASSERT(internalFormat & dataFormat, "Format not supported!");

		glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID);
		glTextureStorage2D(m_RendererID, 1, internalFormat, static_cast<int>(m_Width), static_cast<int>(m_Height));

		/// Set the texture wrapping/filtering options (on the currently bound texture object)
		glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		/// Upload the texture data to the GPU
		glTextureSubImage2D(m_RendererID, 
			0, 
			0, 
			0, 
			static_cast<int>(m_Width), 
			static_cast<int>(m_Width), 
			dataFormat, 
			GL_UNSIGNED_BYTE, 
			data);

		stbi_image_free(data);
	}

	OpenGLTexture2D::OpenGLTexture2D(uint32_t width, uint32_t height) 
	{
		
	}

	OpenGLTexture2D::~OpenGLTexture2D() 
	{
		glDeleteTextures(1, &m_RendererID);
	}

	void OpenGLTexture2D::Bind(const uint32_t slot) const 
	{
		glBindTextureUnit(slot, m_RendererID);
	}

	void OpenGLTexture2D::SetData(void* data, uint32_t size) 
	{
	
	}
}
