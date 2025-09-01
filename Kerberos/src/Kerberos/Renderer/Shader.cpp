#include "kbrpch.h"
#include "Shader.h"
#include "Kerberos/Core.h"
#include "Renderer.h"
#include "Platform/OpenGL/OpenGLShader.h"
#include "Platform/D3D11/D3D11Shader.h"
#include "Platform/Vulkan/VulkanShader.h"

namespace Kerberos
{
	Ref<Shader> Shader::Create(const std::string& filepath)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::OpenGL:
			return CreateRef<OpenGLShader>(filepath);

		case RendererAPI::API::D3D11:
			return CreateRef<D3D11Shader>(filepath);

		case RendererAPI::API::D3D12:
			KBR_CORE_ASSERT(false, "RendererAPI::D3D12 is currently not supported!");
			return nullptr;

		case RendererAPI::API::Vulkan:
			return CreateRef<VulkanShader>(filepath);
		}

		KBR_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

	Ref<Shader> Shader::Create(const std::string& name, const std::string& vertexSrc, const std::string& fragmentSrc)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::OpenGL:
			return CreateRef<OpenGLShader>(name, vertexSrc, fragmentSrc);

		case RendererAPI::API::D3D11:
			return CreateRef<D3D11Shader>(name, vertexSrc, fragmentSrc);

		case RendererAPI::API::D3D12:
			KBR_CORE_ASSERT(false, "RendererAPI::D3D12 is currently not supported!");
			return nullptr;

		case RendererAPI::API::Vulkan:
			return CreateRef<VulkanShader>(name, vertexSrc, fragmentSrc);
		}

		KBR_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

	void ShaderLibrary::Add(const Ref<Shader>& shader)
	{
		auto& name = shader->GetName();
		KBR_CORE_ASSERT(!Exists(name), "Shader already exists!");
		m_Shaders[name] = shader;
	}

	void ShaderLibrary::Add(const std::string& name, const Ref<Shader>& shader)
	{
		KBR_CORE_ASSERT(!Exists(name), "Shader already exists!");
		m_Shaders[name] = shader;
	}

	Ref<Shader> ShaderLibrary::Load(const std::string& filepath)
	{
		auto shader = Shader::Create(filepath);
		Add(shader);
		return shader;
	}

	Ref<Shader> ShaderLibrary::Load(const std::string& name, const std::string& filepath)
	{
		auto shader = Shader::Create(filepath);
		Add(name, shader);
		return shader;
	}

	Ref<Shader> ShaderLibrary::Get(const std::string& name)
	{
		KBR_CORE_ASSERT(Exists(name), "Shader not found!");
		return m_Shaders[name];
	}

	bool ShaderLibrary::Exists(const std::string& name) const
	{
		return m_Shaders.contains(name);
	}
}
