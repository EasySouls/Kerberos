#include "kbrpch.h"
#include "Framebuffer.h"

#include "Renderer.h"
#include "Platform/OpenGL/OpenGLFramebuffer.h"

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

		case RendererAPI::API::Vulkan:
			KBR_CORE_ASSERT(false, "Vulkan is currently not supported!");
			return nullptr;
		}

		KBR_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}
}
