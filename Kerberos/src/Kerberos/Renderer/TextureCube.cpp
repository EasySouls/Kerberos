#include "kbrpch.h"
#include "TextureCube.h"

#include "Renderer.h"
#include "Platform/OpenGL/OpenGLTextureCube.h"
#include "Platform/D3D11/D3D11TextureCube.h"
#include "Platform/Vulkan/VulkanTextureCube.h"

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

		case RendererAPI::API::D3D11:
			return CreateRef<D3D11TextureCube>(name, faces, generateMipmaps, srgb);

		case RendererAPI::API::D3D12:
			KBR_CORE_ASSERT(false, "D3D12 is currently not supported!");
			return nullptr;	

		case RendererAPI::API::Vulkan:
			return CreateRef<VulkanTextureCube>(name, faces, generateMipmaps, srgb);
		}

		KBR_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}
}
