#pragma once

#include "Kerberos/Renderer/Texture.h"

namespace Kerberos::TextureUtils
{
	static constexpr DXGI_FORMAT KBRImageFormatToDXGIFormat(const ImageFormat format)
	{
		switch (format)
		{
		case ImageFormat::RGB8:		return DXGI_FORMAT_R8G8B8A8_UINT;
		case ImageFormat::RGBA8:	return DXGI_FORMAT_R8G8B8A8_UINT;
		case ImageFormat::R8:		return DXGI_FORMAT_R8_TYPELESS;
		case ImageFormat::RGBA32F:	return DXGI_FORMAT_R32G32B32A32_FLOAT;
		case ImageFormat::None:
			break;
		}

		KBR_CORE_ASSERT(false, "KBRImageFormatToGLDataFormat - unsupported format");
		return DXGI_FORMAT_UNKNOWN;
	}
}