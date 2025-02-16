#include "kbrpch.h"

#include "VertexArray.h"
#include "Renderer.h"
#include "Platform/OpenGL/OpenGLVertexArray.h"

namespace Kerberos
{
	VertexArray* VertexArray::Create()
	{
		switch (Renderer::GetAPI())
		{
			case RendererAPI::None:    
				KBR_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); 
				return nullptr;

			case RendererAPI::OpenGL:  
				return new OpenGLVertexArray();
		
			case RendererAPI::Vulkan:
				KBR_CORE_ASSERT(false, "Vulkan is currently not supported!");
				return nullptr;
		}

		KBR_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}
}