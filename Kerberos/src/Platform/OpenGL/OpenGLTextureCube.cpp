#include "kbrpch.h"
#include "OpenGLTextureCube.h"

#include <stb_image.h>


namespace Kerberos 
{
    namespace Utils
    {
        struct Formats
        {
            GLint InternalFormat;
            GLint DataFormat;
        };

        static Formats GetFormats(const int nrChannels, const bool isSrgb)
        {
            if (nrChannels == 4)
            {
                if (isSrgb)
                {
                    return { .InternalFormat = GL_SRGB8, .DataFormat = GL_RGBA };
                }
                return { .InternalFormat = GL_RGBA8, .DataFormat = GL_RGBA };
            }

            if (isSrgb)
            {
	            return { .InternalFormat = GL_SRGB8, .DataFormat = GL_RGB };
            }
            return { .InternalFormat = GL_RGB8, .DataFormat = GL_RGB };
        }
    }

	OpenGLTextureCube::OpenGLTextureCube(std::string name, const std::vector<std::string>& faces,
		const bool generateMipmaps, const bool srgb)
		: m_Name(std::move(name)), m_GenerateMipmaps(generateMipmaps), m_SRGB(srgb)
	{
		KBR_CORE_ASSERT(faces.size() == 6, "OpenGLTextureCube must have 6 faces.");

        glGenTextures(1, &m_RendererID);
        glBindTexture(GL_TEXTURE_CUBE_MAP, m_RendererID);

        int width, height, nrChannels;
        for (unsigned int i = 0; i < faces.size(); i++)
        {
            stbi_uc* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);

            const auto [InternalFormat, DataFormat] = Utils::GetFormats(nrChannels, m_SRGB);

            if (data)
            {
                glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, InternalFormat, width, height, 0, DataFormat, GL_UNSIGNED_BYTE, data);

                stbi_image_free(data);
            }
            else
            {
				KBR_ASSERT(false, "Failed to load texture at path: {0}", faces[i]);
                stbi_image_free(data);
            }
        }
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	}

	OpenGLTextureCube::~OpenGLTextureCube() 
    {
        KBR_PROFILE_FUNCTION();

        glDeleteTextures(1, &m_RendererID);
    }

	void OpenGLTextureCube::Bind(const uint32_t slot) const 
    {
		KBR_PROFILE_FUNCTION();

        KBR_ASSERT(slot < 32, "OpenGLTextureCube slot must be less than 32.");
        glActiveTexture(GL_TEXTURE0 + slot);
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_RendererID);
    }

	uint32_t OpenGLTextureCube::GetWidth() const 
    {
		throw std::runtime_error("OpenGLTextureCube::GetWidth() is not yet implemented.");
    }

	uint32_t OpenGLTextureCube::GetHeight() const 
    {
		throw std::runtime_error("OpenGLTextureCube::GetHeight() is not yet implemented.");
    }

	void OpenGLTextureCube::SetData(void* data, uint32_t size) 
    {
		throw std::runtime_error("OpenGLTextureCube::SetData() is not yet implemented.");
    }
}
