#include "kbrpch.h"
#include "TextureCube.h"

#include "Renderer.h"
#include "Platform/OpenGL/OpenGLTextureCube.h"

namespace Kerberos
{
	Ref<TextureCube> TextureCube::Create(const std::string& name, const std::vector<std::string>& faces,
		bool generateMipmaps, bool srgb) 
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:
			KBR_CORE_ASSERT(false, "RendererAPI::None is currently not supported!");
			return nullptr;

		case RendererAPI::API::OpenGL:
			return CreateRef<OpenGLTextureCube>(name, faces, generateMipmaps, srgb);

		case RendererAPI::API::Vulkan:
			KBR_CORE_ASSERT(false, "Vulkan is currently not supported!");
			return nullptr;
		}

		KBR_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}
}
