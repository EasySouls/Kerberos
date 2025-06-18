#include "kbrpch.h"
#include "OpenGLTexture.h"

#include "stb_image.h"

namespace Kerberos
{
	OpenGLTexture2D::OpenGLTexture2D(const std::string& path)
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

		KBR_ASSERT(internalFormat & dataFormat, "Format not supported!")

		m_InternalFormat = internalFormat;
		m_DataFormat = dataFormat;

		glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID);
		glTextureStorage2D(m_RendererID, 1, internalFormat, static_cast<int>(m_Width), static_cast<int>(m_Height));

		/// Set the texture wrapping/filtering options (on the currently bound texture object)
		glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		/// Upload the texture data to the GPU
		glTextureSubImage2D(m_RendererID, 
			0, 
			0, 
			0, 
			static_cast<int>(m_Width), 
			static_cast<int>(m_Height), 
			dataFormat, 
			GL_UNSIGNED_BYTE, 
			data);

		stbi_image_free(data);
	}

	OpenGLTexture2D::OpenGLTexture2D(const uint32_t width, const uint32_t height)
		: m_Width(width), m_Height(height)
	{
		KBR_PROFILE_FUNCTION();

		/// Internal format is how OpenGl will store the texture data internally (in the GPU)
		constexpr GLenum internalFormat = GL_RGBA8;
		/// Data format is the format of the texture data we provide to OpenGL
		constexpr GLenum dataFormat = GL_RGBA;

		glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID);
		glTextureStorage2D(m_RendererID, 1, internalFormat, static_cast<int>(m_Width), static_cast<int>(m_Height));

		m_InternalFormat = internalFormat;
		m_DataFormat = dataFormat;

		/// Set the texture wrapping/filtering options (on the currently bound texture object)
		glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}

	OpenGLTexture2D::~OpenGLTexture2D() 
	{
		KBR_PROFILE_FUNCTION();

		glDeleteTextures(1, &m_RendererID);
	}

	void OpenGLTexture2D::Bind(const uint32_t slot) const 
	{
		KBR_PROFILE_FUNCTION();

		glBindTextureUnit(slot, m_RendererID);
	}

	void OpenGLTexture2D::SetData(void* data, const uint32_t size) 
	{
		KBR_PROFILE_FUNCTION();

		const uint32_t bytesPerPixel = m_DataFormat == GL_RGBA ? 4 : 3;
		KBR_CORE_ASSERT(size == m_Width * m_Height * bytesPerPixel, "Data must be the entire texture!");

		glTextureSubImage2D(m_RendererID, 0, 0, 0, static_cast<int>(m_Width), static_cast<int>(m_Height), m_DataFormat, GL_UNSIGNED_BYTE, data);
	}
}
