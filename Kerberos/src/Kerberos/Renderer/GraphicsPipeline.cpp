#include "kbrpch.h"
#include "GraphicsPipeline.h"

#include "RendererAPI.h"
#include "Platform/D3D11/D3D11GraphicsPipeline.h"
#include "Platform/Vulkan/VulkanGraphicsPipeline.h"
#include "Platform/OpenGL/OpenGLGraphicsPipeline.h"

namespace Kerberos
{
	Ref<GraphicsPipeline> GraphicsPipeline::Create(const PipelineSpecification& spec) 
	{
		switch (RendererAPI::GetAPI())
		{
		case RendererAPI::API::OpenGL:
			return CreateRef<OpenGLGraphicsPipeline>(spec);

		case RendererAPI::API::Vulkan:
			return CreateRef<VulkanGraphicsPipeline>(spec);

		case RendererAPI::API::D3D11:
			return CreateRef<D3D11GraphicsPipeline>(spec);

		case RendererAPI::API::D3D12:
			//return CreateRef<D3D12Pipeline>(spec);
			KBR_CORE_ASSERT(false, "Pipeline is not yet implemented for D3D12");
			return nullptr;
		}
		KBR_CORE_ASSERT(false, "Unknown RendererAPI for creating a pipeline!");
		return nullptr;
	}
}
