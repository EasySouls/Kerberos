#include "kbrpch.h"
#include "Buffer.h"
#include "Renderer.h"
#include "Kerberos/Core.h"
#include "Platform/OpenGL/OpenGLBuffer.h"

namespace Kerberos
{
	VertexBuffer* VertexBuffer::Create(const float* vertices, const uint32_t size)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::None:
			KBR_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); 
			return nullptr;

		case RendererAPI::OpenGL:
			return new OpenGLVertexBuffer(vertices, size);

		case RendererAPI::Vulkan:
			KBR_CORE_ASSERT(false, "Vulkan is currently not supported!");
			return nullptr;
		}

		KBR_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

	IndexBuffer* IndexBuffer::Create(const uint32_t* indices, const uint32_t count)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::None:
			KBR_CORE_ASSERT(false, "RendererAPI::None is currently not supported!");
			return nullptr;

		case RendererAPI::OpenGL:
			return new OpenGLIndexBuffer(indices, count);

		case RendererAPI::Vulkan:
			KBR_CORE_ASSERT(false, "Vulkan is currently not supported!");
			return nullptr;
		}

		KBR_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}
}
