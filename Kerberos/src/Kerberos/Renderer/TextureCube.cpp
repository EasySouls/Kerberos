#include "kbrpch.h"
#include "TextureCube.h"

#include "Renderer.h"
#include "Platform/OpenGL/OpenGLTextureCube.h"
#include "Platform/D3D11/D3D11TextureCube.h"
#include "Platform/Vulkan/VulkanTextureCube.h"

namespace Kerberos
{
	Ref<TextureCube> TextureCube::Create(const CubemapData& data) 
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::OpenGL:
			return CreateRef<OpenGLTextureCube>(data);

		case RendererAPI::API::D3D11:
			return CreateRef<D3D11TextureCube>(data);

		case RendererAPI::API::D3D12:
			KBR_CORE_ASSERT(false, "D3D12 is currently not supported!");
			return nullptr;

		case RendererAPI::API::Vulkan:
			return CreateRef<VulkanTextureCube>(data);
		}

		KBR_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

}
