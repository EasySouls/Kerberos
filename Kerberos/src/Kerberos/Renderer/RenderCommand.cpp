#include "kbrpch.h"
#include "RenderCommand.h"

#include "RendererAPI.h"
#include "Platform/OpenGL/OpenGLRendererAPI.h"

namespace Kerberos
{
	RendererAPI* RenderCommand::s_RendererAPI = nullptr;

	void RenderCommand::SetupRendererAPI()
	{
		const auto api = RendererAPI::GetAPI();
		switch (api)
		{
		case RendererAPI::API::None:
			KBR_CORE_ASSERT(false, "RendererAPI::None is currently not supported!");
			return;
		case RendererAPI::API::OpenGL:
			s_RendererAPI = new OpenGLRendererAPI();
			return;
		case RendererAPI::API::Vulkan:
			KBR_CORE_ASSERT(false, "RendererAPI::Vulkan is currently not supported!");
			return;
		}

		KBR_CORE_ASSERT(false, "Unknown RendererAPI in RenderCommand::SetupRendererAPI")
	}

}