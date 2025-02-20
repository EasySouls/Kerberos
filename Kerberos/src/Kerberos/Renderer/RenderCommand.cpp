#include "kbrpch.h"
#include "RenderCommand.h"

#include "RendererAPI.h"
#include "Platform/OpenGL/OpenGLRendererAPI.h"
#include "Platform/Vulkan/VulkanRendererAPI.h"

namespace Kerberos
{
	RendererAPI* RenderCommand::s_RendererAPI = nullptr;

	void RenderCommand::SetupRendererAPI()
	{
		switch (const auto api = RendererAPI::GetAPI())
		{
		case RendererAPI::API::None:
			KBR_CORE_ASSERT(false, "RendererAPI::None is currently not supported!");
			return;
		case RendererAPI::API::OpenGL:
			s_RendererAPI = new OpenGLRendererAPI();
			return;
		case RendererAPI::API::Vulkan:
			{
			const auto rendererApi = new VulkanRendererAPI();
			rendererApi->SetupPipeline();
			s_RendererAPI = rendererApi;
			return;
			}
		}

		KBR_CORE_ASSERT(false, "Unknown RendererAPI in RenderCommand::SetupRendererAPI")
	}

}
