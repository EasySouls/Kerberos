#include "kbrpch.h"
#include "Pipeline.h"

#include "RendererAPI.h"

namespace Kerberos
{
	Ref<Pipeline> Pipeline::Create(const PipelineSpecification& spec) 
	{
		switch (RendererAPI::GetAPI())
		{
		case RendererAPI::API::OpenGL:
			KBR_CORE_ASSERT(false, "Pipeline is not yet implemented for OpenGL");
			return nullptr;

		case RendererAPI::API::Vulkan:
			//return CreateRef<VulkanPipeline>(spec);
			KBR_CORE_ASSERT(false, "Pipeline is not yet implemented for Vulkan");
			return nullptr;

		case RendererAPI::API::D3D11:
			//return CreateRef<D3D11Pipeline>(spec);
			KBR_CORE_ASSERT(false, "Pipeline is not yet implemented for D3D11");
			return nullptr;

		case RendererAPI::API::D3D12:
			//return CreateRef<D3D12Pipeline>(spec);
			KBR_CORE_ASSERT(false, "Pipeline is not yet implemented for D3D12");
			return nullptr;
		}
		KBR_CORE_ASSERT(false, "Unknown RendererAPI for creating a pipeline!");
		return nullptr;
	}
}
