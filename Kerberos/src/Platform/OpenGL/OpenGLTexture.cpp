#include "kbrpch.h"
#include "OpenGLTexture.h"

#include "stb_image.h"
#include "TextureUtils.h"

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

		m_Spec.Width = static_cast<unsigned int>(width);
		m_Spec.Height = static_cast<unsigned int>(height);

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
		glTextureStorage2D(m_RendererID, 1, internalFormat, static_cast<int>(m_Spec.Width), static_cast<int>(m_Spec.Height));

		/// Set the texture wrapping/filtering options (on the currently bound texture object)
		glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		/// Upload the texture data to the GPU
		glTextureSubImage2D(m_RendererID, 
			0, 
			0, 
			0, 
			static_cast<int>(m_Spec.Width), 
			static_cast<int>(m_Spec.Height), 
			dataFormat, 
			GL_UNSIGNED_BYTE, 
			data);

		stbi_image_free(data);
	}

	OpenGLTexture2D::OpenGLTexture2D(const TextureSpecification& spec, const Buffer data)
		: m_Spec(spec)
	{
		KBR_PROFILE_FUNCTION();

		/// Internal format is how OpenGl will store the texture data internally (in the GPU)
		const GLenum internalFormat = TextureUtils::KBRImageFormatToGLInternalFormat(spec.Format);
		/// Data format is the format of the texture data we provide to OpenGL
		const GLenum dataFormat = TextureUtils::KBRImageFormatToGLDataFormat(spec.Format);

		glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID);
		glTextureStorage2D(m_RendererID, 1, internalFormat, static_cast<int>(m_Spec.Width), static_cast<int>(m_Spec.Height));

		m_InternalFormat = internalFormat;
		m_DataFormat = dataFormat;

		/// Set the texture wrapping/filtering options (on the currently bound texture object)
		glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		if (data)
		{
			/// Upload the texture data to the GPU
			OpenGLTexture2D::SetData(data.Data, data.Size);
		}
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

		const uint32_t bytesPerPixel = TextureUtils::BytesPerPixel(m_Spec.Format);
		KBR_CORE_ASSERT(size == m_Spec.Width * m_Spec.Height * bytesPerPixel, "Data must be the entire texture!");

		glTextureSubImage2D(m_RendererID, 0, 0, 0, static_cast<int>(m_Spec.Width), static_cast<int>(m_Spec.Height), m_DataFormat, GL_UNSIGNED_BYTE, data);
	}

	void OpenGLTexture2D::SetDebugName(const std::string& name) const {
		KBR_PROFILE_FUNCTION();

		if (m_RendererID)
		{
			glObjectLabel(GL_TEXTURE, m_RendererID, -1, name.c_str());
		}
	}
}
