#include "kbrpch.h"
#include "UniformBuffer.h"

#include "Kerberos/Renderer/Renderer.h"
#include "Platform/OpenGL/OpenGLUniformBuffer.h"
#include "Platform/Vulkan/VulkanUniformBuffer.h"

namespace Kerberos 
{
	Ref<UniformBuffer> UniformBuffer::Create(uint32_t size, uint32_t binding)
	{
		switch (Renderer::GetAPI())
		{
			case RendererAPI::API::OpenGL:  
				return CreateRef<OpenGLUniformBuffer>(size, binding);

			case RendererAPI::API::Vulkan:	
				return CreateRef<VulkanUniformBuffer>(size, binding);

			case RendererAPI::API::D3D11: 
				KBR_CORE_ASSERT(false, "D3D11 Uniform buffer is not yet implemented!"); 
				return nullptr;

			case RendererAPI::API::D3D12:
				KBR_CORE_ASSERT(false, "D3D12 Uniform buffer is not yet implemented!"); 
				return nullptr;
		}

		KBR_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}
}
