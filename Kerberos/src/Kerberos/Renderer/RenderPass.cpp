#include "kbrpch.h"

#include "RenderPass.h"
#include "RendererAPI.h"
#include "Platform/Vulkan/VulkanRenderPass.h"
#include "Platform/D3D11/D3D11RenderPass.h"
#include "Platform/OpenGL/OpenGLRenderPass.h"

namespace Kerberos
{
	Ref<RenderPass> RenderPass::Create(const RenderPassSpecification& spec)
	{
		switch (RendererAPI::GetAPI())
		{
		case RendererAPI::API::OpenGL:
			return CreateRef<OpenGLRenderPass>(spec);

		case RendererAPI::API::Vulkan:  
			return CreateRef<VulkanRenderPass>(spec);

		case RendererAPI::API::D3D11:
			return CreateRef<D3D11RenderPass>(spec);

		case RendererAPI::API::D3D12:
			//return CreateRef<D3D12RenderPass>(spec);
			KBR_CORE_ASSERT(false, "RenderPass is not yet implemented for D3D12");
			return nullptr;
		}
		KBR_CORE_ASSERT(false, "Unknown RendererAPI for creating a renderpass!");
		return nullptr;
	}
}
