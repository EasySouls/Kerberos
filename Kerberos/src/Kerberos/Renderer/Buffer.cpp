#include "kbrpch.h"
#include "Buffer.h"
#include "RendererAPI.h"
#include "Kerberos/Core.h"
#include "Platform/D3D11/D3D11Buffer.h"
#include "Platform/OpenGL/OpenGLBuffer.h"
#include "Platform/Vulkan/VulkanBuffer.h"

namespace Kerberos
{
	Ref<VertexBuffer> VertexBuffer::Create(const float* vertices, const uint32_t size)
	{
		switch (RendererAPI::GetAPI())
		{
		case RendererAPI::API::OpenGL:
			return CreateRef<OpenGLVertexBuffer>(vertices, size);

		case RendererAPI::API::D3D11:
			return CreateRef<D3D11VertexBuffer>(vertices, size);

		case RendererAPI::API::D3D12:
			KBR_CORE_ASSERT(false, "D3D12 is currently not supported!");
			return nullptr;

		case RendererAPI::API::Vulkan:
			return CreateRef<VulkanVertexBuffer>(vertices, size);
		}

		KBR_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

	Ref<VertexBuffer> VertexBuffer::Create(uint32_t size)
	{
		switch (RendererAPI::GetAPI())
		{
		case RendererAPI::API::OpenGL:
			return CreateRef<OpenGLVertexBuffer>(size);

		case RendererAPI::API::D3D11:
			return CreateRef<D3D11VertexBuffer>(size);

		case RendererAPI::API::D3D12:
			KBR_CORE_ASSERT(false, "D3D12 is currently not supported!");
			return nullptr;

		case RendererAPI::API::Vulkan:
			return CreateRef<VulkanVertexBuffer>(size);
		}

		KBR_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

	Ref<IndexBuffer> IndexBuffer::Create(const uint32_t* indices, const uint32_t count)
	{
		switch (RendererAPI::GetAPI())
		{
		case RendererAPI::API::OpenGL:
			return CreateRef<OpenGLIndexBuffer>(indices, count);

		case RendererAPI::API::D3D11:
			return CreateRef<D3D11IndexBuffer>(indices, count);

		case RendererAPI::API::D3D12:
			KBR_CORE_ASSERT(false, "D3D12 is currently not supported!");
			return nullptr;

		case RendererAPI::API::Vulkan:
			return CreateRef<VulkanIndexBuffer>(indices, count);
		}

		KBR_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}
}
