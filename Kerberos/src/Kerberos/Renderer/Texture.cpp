#include "kbrpch.h"
#include "Texture.h"
#include "Renderer.h"
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
			return std::make_shared<OpenGLTexture2D>(path);

		case RendererAPI::API::Vulkan:
			KBR_CORE_ASSERT(false, "Vulkan is currently not supported!");
			return nullptr;
		}

		KBR_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}
}
