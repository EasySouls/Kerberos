#include "kbrpch.h"
#include "Shader.h"
#include "Kerberos/Core.h"
#include "Renderer.h"
#include "Platform/OpenGL/OpenGLShader.h"


namespace Kerberos
{
	Shader* Shader::Create(const std::string& filepath) 
	{
		switch (Renderer::GetAPI())
		{
			case RendererAPI::API::None:
				KBR_CORE_ASSERT(false, "RendererAPI::None is currently not supported!")
					return nullptr;
			case RendererAPI::API::OpenGL:
				return new OpenGLShader(filepath);
			case RendererAPI::API::Vulkan:
				KBR_CORE_ASSERT(false, "Vulkan is currently not supported!")
					return nullptr;
		}

		KBR_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

	Shader* Shader::Create(const std::string& vertexSrc, const std::string& fragmentSrc) 
	{
		switch (Renderer::GetAPI())
		{
			case RendererAPI::API::None:    
				KBR_CORE_ASSERT(false, "RendererAPI::None is currently not supported!")
				return nullptr;
			case RendererAPI::API::OpenGL:  
				return new OpenGLShader(vertexSrc, fragmentSrc);
			case RendererAPI::API::Vulkan:
				KBR_CORE_ASSERT(false, "Vulkan is currently not supported!")
					return nullptr;
		}

		KBR_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}
}
