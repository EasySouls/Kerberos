#include "kbrpch.h"
#include "RenderCommand.h"

#include "RendererAPI.h"
#include "Platform/D3D11/D3D11RendererAPI.h"
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

		case RendererAPI::API::D3D11:
			s_RendererAPI = new D3D11RendererAPI();
			return;

		case RendererAPI::API::D3D12:
			KBR_CORE_ASSERT(false, "D3D12 is not implemented yet!");
			return;

		case RendererAPI::API::Vulkan:
			{
			s_RendererAPI = new VulkanRendererAPI();
			return;
			}
		}

		KBR_CORE_ASSERT(false, "Unknown RendererAPI in RenderCommand::SetupRendererAPI")
	}

}
