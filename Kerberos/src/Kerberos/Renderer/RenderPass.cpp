#include "kbrpch.h"

#include "RenderPass.h"
#include "RendererAPI.h"

namespace Kerberos
{
	Ref<RenderPass> RenderPass::Create(const RenderPassSpecification& spec)
	{
		switch (RendererAPI::GetAPI())
		{
		case RendererAPI::API::OpenGL:  
			KBR_CORE_ASSERT(false, "RenderPass is not yet implemented for OpenGL"); 
			return nullptr;

		case RendererAPI::API::Vulkan:  
			//return CreateRef<VulkanRenderPass>(spec);
			KBR_CORE_ASSERT(false, "RenderPass is not yet implemented for Vulkan");
			return nullptr;

		case RendererAPI::API::D3D11:
			//return CreateRef<D3D11RenderPass>(spec);
			KBR_CORE_ASSERT(false, "RenderPass is not yet implemented for D3D11");
			return nullptr;

		case RendererAPI::API::D3D12:
			//return CreateRef<D3D12RenderPass>(spec);
			KBR_CORE_ASSERT(false, "RenderPass is not yet implemented for D3D12");
			return nullptr;
		}
		KBR_CORE_ASSERT(false, "Unknown RendererAPI for creating a renderpass!");
		return nullptr;
	}
}
