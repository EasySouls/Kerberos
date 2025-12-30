#include "kbrpch.h"
#include "CommandList.h"

#include "Renderer.h"
#include "Platform/Vulkan/VulkanCommandList.h"

namespace Kerberos
{
	Ref<CommandList> CommandList::Create()
	{
		switch (Renderer::GetAPI())
		{
			case RendererAPI::API::OpenGL:
				KBR_CORE_ASSERT(false, "OpenGL doesn't yet support command lists!");
				return nullptr;

			case RendererAPI::API::D3D11:
				KBR_CORE_ASSERT(false, "D3D11 is currently not supported!");
				return nullptr;

			case RendererAPI::API::D3D12:
				KBR_CORE_ASSERT(false, "D3D12 is currently not supported!");
				return nullptr;

			case RendererAPI::API::Vulkan:
				return CreateRef<VulkanCommandList>();
		}

		KBR_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}
}