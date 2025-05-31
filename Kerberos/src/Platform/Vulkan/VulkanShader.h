#pragma once


#include "Kerberos/Renderer/Shader.h"

#include <vulkan/vulkan.h>

#include "glad/glad.h"

namespace Kerberos
{
	class VulkanShader final : public Shader
	{
	public:
		explicit VulkanShader(const std::string& filepath);
		VulkanShader(std::string name, const std::string& vertexSrc, const std::string& fragmentSrc, const std::string& geometrySrc = "");
		~VulkanShader() override;

		void Bind() const override;
		void Unbind() const override;
		const std::string& GetName() const override;

		void SetInt(const std::string& name, int value) override;
		void SetIntArray(const std::string& name, int* values, uint32_t count) override;
		void SetFloat(const std::string& name, float value) override;
		void SetFloat3(const std::string& name, const glm::vec3& value) override;
		void SetFloat4(const std::string& name, const glm::vec4& value) override;
		void SetMat4(const std::string& name, const glm::mat4& value) override;
		void SetMaterial(const std::string& name, const Ref<Material>& material) override;

		std::unordered_map<VkShaderStageFlagBits, VkShaderModule>& GetShaderModules() { return m_ShaderModules; }
		std::unordered_map<VkShaderStageFlagBits, VkPipelineShaderStageCreateInfo>& GetPipelineShaderStageCreateInfos() { return m_PipelineShaderStageCreateInfos; }
		const std::vector<VkDescriptorSetLayout>& GetDescriptorSetLayouts() const { return m_DescriptorSetLayouts; }
		const std::vector<VkPushConstantRange>& GetPushConstantRanges() const { return m_PushConstantRanges; }

	private:
		static std::string ReadShaderFile(const std::string& filename);
		static std::unordered_map<GLenum, std::string> SplitShaderSource(const std::string& source);

		void CompileOrGetVulkanBinaries(const std::unordered_map<GLenum, std::string>& shaderSources);

		void CreateShaderModules();

		void Reflect(VkShaderStageFlagBits stage, const std::vector<uint32_t>& spirvCode);
		void ReflectAllStages();

		void CreateDescriptorSetLayouts();

	private:
		std::string m_Name;
		std::string m_Filepath;

		std::unordered_map<GLenum, std::vector<uint32_t>> m_VulkanSPIRV;

		std::unordered_map<VkShaderStageFlagBits, VkShaderModule> m_ShaderModules;
		std::unordered_map<VkShaderStageFlagBits, VkPipelineShaderStageCreateInfo> m_PipelineShaderStageCreateInfos;

		std::map<uint32_t, std::map<uint32_t, VkDescriptorSetLayoutBinding>> m_DescriptorSetLayoutsInfo;
		std::vector<VkDescriptorSetLayout> m_DescriptorSetLayouts;
		std::vector<VkPushConstantRange> m_PushConstantRanges;
	};
}
