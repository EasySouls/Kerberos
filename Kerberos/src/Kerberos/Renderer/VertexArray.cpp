#include "kbrpch.h"

#include "VertexArray.h"
#include "Renderer.h"
#include "Platform/OpenGL/OpenGLVertexArray.h"
#include "Platform/D3D11/D3D11VertexArray.h"
#include "Platform/Vulkan/VulkanVertexArray.h"

namespace Kerberos
{
	Ref<VertexArray> VertexArray::Create()
	{
		switch (Renderer::GetAPI())
		{
			case RendererAPI::API::None:    
				KBR_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); 
				return nullptr;

			case RendererAPI::API::OpenGL:  
				return CreateRef<OpenGLVertexArray>();

			case RendererAPI::API::D3D11:
				return CreateRef<D3D11VertexArray>();

			case RendererAPI::API::D3D12:
				KBR_CORE_ASSERT(false, "D3D12 is currently not supported!");
				return nullptr;
		
			case RendererAPI::API::Vulkan:
				return CreateRef<VulkanVertexArray>();
		}

		KBR_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}
}
