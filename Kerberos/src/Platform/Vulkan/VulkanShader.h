#pragma once


#include "Kerberos/Renderer/Shader.h"

#include <vulkan/vulkan.h>
#include <glslang/Public/ShaderLang.h>

namespace Kerberos
{
	class VulkanShader final : public Shader
	{
	public:
		void Bind() const override;
		void Unbind() const override;
		const std::string& GetName() const override;

	private:
		static std::string ReadShaderFile(const std::string& filename);
		static std::pair<std::string, std::string> SplitShaderSource(const std::string& source);
		static std::vector<uint32_t> CompileGLSLToSPIRV(const std::string& glslSource, EShLanguage shaderType);
		static VkShaderModule CreateShaderModule(VkDevice device, const std::vector<uint32_t>& spirvCode);

		static TBuiltInResource InitResources();
	};
}
