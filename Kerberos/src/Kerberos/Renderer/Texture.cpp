#include "kbrpch.h"
#include "Texture.h"
#include "Renderer.h"
#include "Kerberos/Core.h"
#include "Platform/D3D11/D3D11Texture.h"
#include "Platform/OpenGL/OpenGLTexture.h"
#include "Platform/Vulkan/VulkanTexture.h"

namespace Kerberos
{
	Ref<Texture2D> Texture2D::Create(const std::string& path) 
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:
			KBR_CORE_ASSERT(false, "RendererAPI::None is currently not supported!");
			return nullptr;

		case RendererAPI::API::OpenGL:
			return CreateRef<OpenGLTexture2D>(path);

		case RendererAPI::API::D3D11:
			return CreateRef<D3D11Texture2D>(path);

		case RendererAPI::API::D3D12:
			KBR_CORE_ASSERT(false, "D3D12 is not implemented yet!");
			return nullptr;

		case RendererAPI::API::Vulkan:
			return CreateRef<VulkanTexture2D>(path);
		}

		KBR_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

	Ref<Texture2D> Texture2D::Create(uint32_t width, uint32_t height) 
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:
			KBR_CORE_ASSERT(false, "RendererAPI::None is currently not supported!");
			return nullptr;

		case RendererAPI::API::OpenGL:
			return CreateRef<OpenGLTexture2D>(width, height);

		case RendererAPI::API::D3D11:
			return CreateRef<D3D11Texture2D>(width, height);

		case RendererAPI::API::D3D12:
			KBR_CORE_ASSERT(false, "D3D12 is not implemented yet!");
			return nullptr;

		case RendererAPI::API::Vulkan:
			return CreateRef<VulkanTexture2D>(width, height);
		}

		KBR_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}
}
