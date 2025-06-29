#include "kbrpch.h"
#include "Texture.h"
#include "Renderer.h"
#include "Kerberos/Core.h"
#include "Platform/D3D11/D3D11Texture.h"
#include "Platform/OpenGL/OpenGLTexture.h"
#include "Platform/Vulkan/VulkanTexture.h"

namespace Kerberos
{
	Ref<Texture2D> Texture2D::Create(const TextureSpecification& spec, Buffer data)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:
			KBR_CORE_ASSERT(false, "RendererAPI::None is currently not supported!");
			return nullptr;

		case RendererAPI::API::OpenGL:
			return CreateRef<OpenGLTexture2D>(spec, data);

		case RendererAPI::API::D3D11:
			return CreateRef<D3D11Texture2D>(spec, data);

		case RendererAPI::API::D3D12:
			KBR_CORE_ASSERT(false, "D3D12 is not implemented yet!");
			return nullptr;

		case RendererAPI::API::Vulkan:
			return CreateRef<VulkanTexture2D>(spec, data);
		}

		KBR_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}
}
