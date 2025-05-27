#include "kbrpch.h"

#include "VertexArray.h"
#include "Renderer.h"
#include "Platform/OpenGL/OpenGLVertexArray.h"
#include "Platform/D3D11/D3D11VertexArray.h"

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
				return std::make_shared<OpenGLVertexArray>();

			case RendererAPI::API::D3D11:
				return std::make_shared<D3D11VertexArray>();

			case RendererAPI::API::D3D12:
				KBR_CORE_ASSERT(false, "D3D12 is currently not supported!");
				return nullptr;
		
			case RendererAPI::API::Vulkan:
				KBR_CORE_ASSERT(false, "Vulkan is currently not supported!");
				return nullptr;
		}

		KBR_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}
}