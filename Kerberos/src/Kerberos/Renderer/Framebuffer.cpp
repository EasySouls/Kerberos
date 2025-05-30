#include "kbrpch.h"
#include "Framebuffer.h"

#include "Renderer.h"
#include "Platform/D3D11/D3D11Framebuffer.h"
#include "Platform/OpenGL/OpenGLFramebuffer.h"
#include "Platform/Vulkan/VulkanFramebuffer.h"

namespace Kerberos
{
	Ref<Framebuffer> Framebuffer::Create(const FramebufferSpecification& spec) 
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:
			KBR_CORE_ASSERT(false, "RendererAPI::None is currently not supported!");
			return nullptr;

		case RendererAPI::API::OpenGL:
			return CreateRef<OpenGLFramebuffer>(spec);

		case RendererAPI::API::D3D11:
			return CreateRef<D3D11Framebuffer>(spec);

		case RendererAPI::API::D3D12:
			KBR_CORE_ASSERT(false, "D3D12 is currently not supported!");
			return nullptr;

		case RendererAPI::API::Vulkan:
			return CreateRef<VulkanFramebuffer>(spec);
		}

		KBR_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}
}
