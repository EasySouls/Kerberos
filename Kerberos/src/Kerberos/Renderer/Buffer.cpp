#include "kbrpch.h"
#include "Buffer.h"
#include "RendererAPI.h"
#include "Kerberos/Core.h"
#include "Platform/OpenGL/OpenGLBuffer.h"

namespace Kerberos
{
	Ref<VertexBuffer> VertexBuffer::Create(const float* vertices, const uint32_t size)
	{
		switch (RendererAPI::GetAPI())
		{
			case RendererAPI::API::None:
				KBR_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); 
				return nullptr;

			case RendererAPI::API::OpenGL:
				return CreateRef<OpenGLVertexBuffer>(vertices, size);

			case RendererAPI::API::Vulkan:
				KBR_CORE_ASSERT(false, "Vulkan is currently not supported!");
				return nullptr;
		}

		KBR_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

	Ref<VertexBuffer> VertexBuffer::Create(uint32_t size) 
	{
		switch (RendererAPI::GetAPI())
		{
			case RendererAPI::API::None:
				KBR_CORE_ASSERT(false, "RendererAPI::None is currently not supported!");
				return nullptr;

			case RendererAPI::API::OpenGL:
				return CreateRef<OpenGLVertexBuffer>(size);

			case RendererAPI::API::Vulkan:
				KBR_CORE_ASSERT(false, "Vulkan is currently not supported!");
				return nullptr;
		}

		KBR_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

	IndexBuffer* IndexBuffer::Create(const uint32_t* indices, const uint32_t count)
	{
		switch (RendererAPI::GetAPI())
		{
			case RendererAPI::API::None:
				KBR_CORE_ASSERT(false, "RendererAPI::None is currently not supported!");
				return nullptr;

			case RendererAPI::API::OpenGL:
				return new OpenGLIndexBuffer(indices, count);

			case RendererAPI::API::Vulkan:
				KBR_CORE_ASSERT(false, "Vulkan is currently not supported!");
				return nullptr;
		}

		KBR_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}
}
