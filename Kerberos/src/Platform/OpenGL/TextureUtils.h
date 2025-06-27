#pragma once
#include "glad/glad.h"
#include "Kerberos/Renderer/Texture.h"

namespace Kerberos::TextureUtils
{
	static constexpr GLenum KBRImageFormatToGLDataFormat(const ImageFormat format)
	{
		switch (format)
		{
		case ImageFormat::RGB8: return GL_RGB;
		case ImageFormat::RGBA8: return GL_RGBA;
		case ImageFormat::None:
		case ImageFormat::R8:
		case ImageFormat::RGBA32F:
			break;
		}

		KBR_CORE_ASSERT(false, "KBRImageFormatToGLDataFormat - unsupported format");
		return 0;
	}

	static constexpr GLenum KBRImageFormatToGLInternalFormat(const ImageFormat format)
	{
		switch (format)
		{
		case ImageFormat::RGB8: return GL_RGB8;
		case ImageFormat::RGBA8: return GL_RGBA8;
		case ImageFormat::None:
		case ImageFormat::R8:
		case ImageFormat::RGBA32F:
			break;
		}

		KBR_CORE_ASSERT(false, "KBRImageFormatToGLInternalFormat - unsupported format");
		return 0;
	}
}
