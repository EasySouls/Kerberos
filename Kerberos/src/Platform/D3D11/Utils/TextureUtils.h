#pragma once

#include "Kerberos/Renderer/Framebuffer.h"
#include "Kerberos/Renderer/Texture.h"


namespace Kerberos::TextureUtils
{
	static constexpr DXGI_FORMAT KBRImageFormatToDXGITextureFormat(const ImageFormat format)
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

	/** 
	 *	Returns the appropriate DXGI_FORMAT for a given FramebufferTextureFormat.
	 *  Note: For depth formats, this returns a TYPELESS format suitable for creating the texture.
	 */
	constexpr DXGI_FORMAT GetTextureFormat(const FramebufferTextureFormat format)
	{
		switch (format)
		{
		case FramebufferTextureFormat::RGBA8:
			return DXGI_FORMAT_R8G8B8A8_UNORM;

		case FramebufferTextureFormat::RED_INTEGER:
			return DXGI_FORMAT_R32_SINT;

			// DEPTH24: DX11 does not have a pure 24-bit depth format. 
			// We must use 24-bit depth + 8-bit stencil (even if stencil is unused).
		case FramebufferTextureFormat::DEPTH24STENCIL8:
		case FramebufferTextureFormat::DEPTH24:
			return DXGI_FORMAT_R24G8_TYPELESS;

		case FramebufferTextureFormat::DEPTH32:
			return DXGI_FORMAT_R32_TYPELESS;

		case FramebufferTextureFormat::None:
			break;
		}

		KBR_CORE_ASSERT(false, "GetTextureFormat - unsupported format");
		return DXGI_FORMAT_UNKNOWN;
	}

	/** 
	 *	Returns the appropriate DXGI_FORMAT for creating a Shader Resource View (SRV)
	 *	for a given FramebufferTextureFormat.
	*/
	constexpr DXGI_FORMAT GetSRVFormat(const FramebufferTextureFormat format)
	{
		switch (format)
		{
		case FramebufferTextureFormat::RGBA8:
			return DXGI_FORMAT_R8G8B8A8_UNORM;

		case FramebufferTextureFormat::RED_INTEGER:
			return DXGI_FORMAT_R32_SINT;

		case FramebufferTextureFormat::DEPTH24STENCIL8:
		case FramebufferTextureFormat::DEPTH24:
			// Read the Depth component (Red channel) as UNORM
			return DXGI_FORMAT_R24_UNORM_X8_TYPELESS;

		case FramebufferTextureFormat::DEPTH32:
			// Read the Depth component as Float
			return DXGI_FORMAT_R32_FLOAT;

		case FramebufferTextureFormat::None:
			break;
		}

		KBR_CORE_ASSERT(false, "GetSRVFormat - unsupported format");
		return DXGI_FORMAT_UNKNOWN;
	}

	/** 
	 *	Returns the appropriate DXGI_FORMAT for creating a Depth Stencil View (DSV)
	 *	for a given FramebufferTextureFormat.
	*/
	constexpr DXGI_FORMAT GetDSVFormat(const FramebufferTextureFormat format)
	{
		switch (format)
		{
			// Color formats cannot be used for Depth Stencil Views
		case FramebufferTextureFormat::RGBA8:
		case FramebufferTextureFormat::RED_INTEGER:
			KBR_CORE_ASSERT(false, "GetDSVFormat - format is not a depth format");
			return DXGI_FORMAT_UNKNOWN;

		case FramebufferTextureFormat::DEPTH24STENCIL8:
		case FramebufferTextureFormat::DEPTH24:
			return DXGI_FORMAT_D24_UNORM_S8_UINT;

		case FramebufferTextureFormat::DEPTH32:
			return DXGI_FORMAT_D32_FLOAT;

		case FramebufferTextureFormat::None:
			break;
		}

		KBR_CORE_ASSERT(false, "GetDSVFormat - unsupported format");
		return DXGI_FORMAT_UNKNOWN;
	}

	/** 
	 *	Returns the appropriate DXGI_FORMAT for creating a Render Target View (RTV)
	 *	for a given FramebufferTextureFormat.
	*/
	constexpr DXGI_FORMAT GetRTVFormat(const FramebufferTextureFormat format)
	{
		switch (format)
		{
		case FramebufferTextureFormat::RGBA8:
			return DXGI_FORMAT_R8G8B8A8_UNORM;

		case FramebufferTextureFormat::RED_INTEGER:
			return DXGI_FORMAT_R32_SINT;

			// Depth formats cannot be used for Render Target Views
		case FramebufferTextureFormat::DEPTH24STENCIL8:
		case FramebufferTextureFormat::DEPTH24:
		case FramebufferTextureFormat::DEPTH32:
			KBR_CORE_ASSERT(false, "GetRTVFormat - format cannot be used for rendering output");
			return DXGI_FORMAT_UNKNOWN;

		case FramebufferTextureFormat::None:
			break;
		}

		KBR_CORE_ASSERT(false, "GetRTVFormat - unsupported format");
		return DXGI_FORMAT_UNKNOWN;
	}
}