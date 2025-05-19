#include "kbrpch.h"
#include "Texture.h"
#include "Renderer.h"
#include "Kerberos/Core.h"
#include "Platform/OpenGL/OpenGLTexture.h"

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

		case RendererAPI::API::Vulkan:
			KBR_CORE_ASSERT(false, "Vulkan is currently not supported!");
			return nullptr;
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

		case RendererAPI::API::Vulkan:
			KBR_CORE_ASSERT(false, "Vulkan is currently not supported!");
			return nullptr;
		}

		KBR_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}
}
