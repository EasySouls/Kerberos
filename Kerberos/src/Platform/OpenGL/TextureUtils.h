#pragma once
#include "glad/glad.h"
#include "Kerberos/Renderer/Texture.h"

namespace Kerberos::TextureUtils
{
	static constexpr GLenum KBRImageFormatToGLDataFormat(const ImageFormat format)
	{
		switch (format)
		{
		case ImageFormat::RGB8:		return GL_RGB;
		case ImageFormat::RGBA8:	return GL_RGBA;
		case ImageFormat::R8:		return GL_RED;
		case ImageFormat::None:
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
		case ImageFormat::RGB8:		return GL_RGB8;
		case ImageFormat::RGBA8:	return GL_RGBA8;
		case ImageFormat::R8:		return GL_R8;
		case ImageFormat::None:
		case ImageFormat::RGBA32F:
			break;
		}

		KBR_CORE_ASSERT(false, "KBRImageFormatToGLInternalFormat - unsupported format");
		return 0;
	}

	static constexpr uint32_t BytesPerPixel(const ImageFormat format)
	{
		switch (format)
		{
		case ImageFormat::RGB8:		return 3;
		case ImageFormat::RGBA8:	return 4;
		case ImageFormat::R8:		return 1;
		case ImageFormat::RGBA32F:	return 16; // 4 floats, 4 bytes each
		case ImageFormat::None:
			break;
		}
		KBR_CORE_ASSERT(false, "BytesPerPixel - unsupported format");
		return 0;
	}
}
