#include "kbrpch.h"
#include "VulkanShader.h"

#include <filesystem>

#include <shaderc/shaderc.hpp>
#include <spirv_cross/spirv_cross.hpp>
#include <spirv_cross/spirv_glsl.hpp>

#include "VulkanContext.h"

namespace Kerberos
{
	namespace Utils
	{
		static shaderc_shader_kind GLShaderStageToShaderC(const GLenum stage) {
			switch (stage)
			{
			case GL_VERTEX_SHADER:   return shaderc_glsl_vertex_shader;
			case GL_FRAGMENT_SHADER: return shaderc_glsl_fragment_shader;
			case GL_GEOMETRY_SHADER: return shaderc_glsl_geometry_shader;
			}
			KBR_CORE_ASSERT(false, "Unknown shader stage for Vulkan!");
			return static_cast<shaderc_shader_kind>(0);
		}

		static VkShaderStageFlagBits GLShaderStageToVulkanStage(const GLenum stage) {
			switch (stage)
			{
			case GL_VERTEX_SHADER:   return VK_SHADER_STAGE_VERTEX_BIT;
			case GL_FRAGMENT_SHADER: return VK_SHADER_STAGE_FRAGMENT_BIT;
			case GL_GEOMETRY_SHADER: return VK_SHADER_STAGE_GEOMETRY_BIT;
			}
			KBR_CORE_ASSERT(false, "Unknown shader stage for Vulkan!");
			return static_cast<VkShaderStageFlagBits>(0);
		}

		static const char* GLShaderStageToString(const GLenum stage) {
			switch (stage) {
			case GL_VERTEX_SHADER:   return "GL_VERTEX_SHADER";
			case GL_FRAGMENT_SHADER: return "GL_FRAGMENT_SHADER";
			case GL_GEOMETRY_SHADER: return "GL_GEOMETRY_SHADER";
			}
			KBR_CORE_ASSERT(false, "Unknown or unsupported shader stage");
			return nullptr;
		}

		static const char* GetCacheDirectory() {
			return "assets/cache/shader/vulkan";
		}

		static void CreateCacheDirectoryIfNeeded() {
			const std::string cacheDirectory = GetCacheDirectory();
			if (!std::filesystem::exists(cacheDirectory))
				std::filesystem::create_directories(cacheDirectory);
		}

		static const char* ShaderStageCachedFileExtension(const GLenum stage) {
			switch (stage) {
			case GL_VERTEX_SHADER:    return ".cached_vulkan.vert.spv";
			case GL_FRAGMENT_SHADER:  return ".cached_vulkan.frag.spv";
			case GL_GEOMETRY_SHADER:  return ".cached_vulkan.geom.spv";
			}
			KBR_CORE_ASSERT(false, "Unknown or unsupported shader stage");
			return "";
		}
	}

	VulkanShader::VulkanShader(const std::string& filepath)
		: m_Filepath(filepath)
	{
		KBR_PROFILE_FUNCTION();

		Utils::CreateCacheDirectoryIfNeeded();

		const std::string source = ReadShaderFile(filepath);
		const auto shaderSources = SplitShaderSource(source);

		{
			// TODO: Time shader compilation and caching
			CompileOrGetVulkanBinaries(shaderSources);
			CreateShaderModules();
			ReflectAllStages();
		}

		auto lastSlash = filepath.find_last_of("/\\");
		lastSlash = lastSlash == std::string::npos ? 0 : lastSlash + 1;
		const auto lastDot = filepath.rfind('.');
		const auto count = lastDot == std::string::npos ? filepath.size() - lastSlash : lastDot - lastSlash;
		m_Name = filepath.substr(lastSlash, count);
	}

	VulkanShader::VulkanShader(std::string name, const std::string& vertexSrc, const std::string& fragmentSrc,
		const std::string& geometrySrc)
		: m_Name(std::move(name))
	{
		KBR_PROFILE_FUNCTION();

		std::unordered_map<GLenum, std::string> sources;
		sources[GL_VERTEX_SHADER] = vertexSrc;
		sources[GL_FRAGMENT_SHADER] = fragmentSrc;

		CompileOrGetVulkanBinaries(sources);
		CreateShaderModules();
		ReflectAllStages();
	}

	VulkanShader::~VulkanShader()
	{
		KBR_PROFILE_FUNCTION();

		const VkDevice device = VulkanContext::Get().GetDevice();

		KBR_CORE_ASSERT(device, "Vulkan logical device is null in destructor!");

		for (auto& kv : m_ShaderModules) {
			if (kv.second != VK_NULL_HANDLE) {
				vkDestroyShaderModule(device, kv.second, nullptr);
			}
		}
		m_ShaderModules.clear();
		m_PipelineShaderStageCreateInfos.clear();
	}

	void VulkanShader::Bind() const
	{

	}

	void VulkanShader::Unbind() const
	{

	}

	const std::string& VulkanShader::GetName() const
	{
		return m_Name;
	}

	void VulkanShader::SetInt(const std::string& name, int value)
	{

	}

	void VulkanShader::SetIntArray(const std::string& name, int* values, uint32_t count)
	{

	}

	void VulkanShader::SetFloat(const std::string& name, float value)
	{

	}

	void VulkanShader::SetFloat3(const std::string& name, const glm::vec3& value)
	{

	}

	void VulkanShader::SetFloat4(const std::string& name, const glm::vec4& value)
	{

	}

	void VulkanShader::SetMat4(const std::string& name, const glm::mat4& value)
	{

	}

	void VulkanShader::SetMaterial(const std::string& name, const Ref<Material>& material)
	{

	}

	std::string VulkanShader::ReadShaderFile(const std::string& filename)
	{
		KBR_PROFILE_FUNCTION();

		const std::ifstream file(filename);
		if (!file.is_open())
		{
			throw std::runtime_error("Failed to open shader file: " + filename);
		}
		std::stringstream buffer;
		buffer << file.rdbuf();
		return buffer.str();
	}

	std::unordered_map<GLenum, std::string> VulkanShader::SplitShaderSource(const std::string& source)
	{
		KBR_PROFILE_FUNCTION();

		/// TODO: This should look for #type directive to determine the shader stages

		const size_t vertexPos = source.find("#type vertex");
		const size_t fragmentPos = source.find("#type fragment");
		const size_t geometryPos = source.find("#type geometry");

		if (vertexPos == std::string::npos || fragmentPos == std::string::npos)
		{
			throw std::runtime_error(
				"Shader file must contain '#type vertex' and '#type fragment' "
				"directives.");
		}

		const std::string vertexSource = source.substr(vertexPos + std::string("#type vertex").length(),
			fragmentPos - vertexPos -
			std::string("#type vertex").length());
		const std::string fragmentSource = source.substr(fragmentPos +
			std::string("#type fragment").length());

		std::unordered_map<GLenum, std::string> shaderSources;
		shaderSources[GL_VERTEX_SHADER] = vertexSource;
		shaderSources[GL_FRAGMENT_SHADER] = fragmentSource;

		return shaderSources;
	}

	void VulkanShader::CompileOrGetVulkanBinaries(const std::unordered_map<GLenum, std::string>& shaderSources)
	{
		KBR_PROFILE_FUNCTION();

		shaderc::Compiler compiler;
		shaderc::CompileOptions options;
		options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_4);

		if (constexpr bool optimize = true)
			options.SetOptimizationLevel(shaderc_optimization_level_performance);

		std::filesystem::path cacheDirectory = Utils::GetCacheDirectory();

		m_VulkanSPIRV.clear();

		for (auto&& [stage, source] : shaderSources) {
			std::filesystem::path shaderFilePath = m_Filepath;

			std::string baseFilename = shaderFilePath.empty() ? m_Name : shaderFilePath.filename().string();
			if (baseFilename.empty()) baseFilename = "unnamed_shader";

			std::filesystem::path cachedPath = cacheDirectory / (baseFilename + Utils::ShaderStageCachedFileExtension(stage));

			std::ifstream in(cachedPath, std::ios::in | std::ios::binary);
			if (in.is_open()) {
				/// The file exists, read the SPIR-V binary from cache
				
				in.seekg(0, std::ios::end);
				auto size = in.tellg();
				in.seekg(0, std::ios::beg);

				auto& data = m_VulkanSPIRV[stage];
				data.resize(size / sizeof(uint32_t));
				in.read(reinterpret_cast<char*>(data.data()), size);
				in.close();
				KBR_CORE_TRACE("Vulkan Shader: Loaded SPIR-V from cache: {0}", cachedPath.string());
			}
			else {
				/// The file does not exist, compile the GLSL source to SPIR-V

				shaderc::SpvCompilationResult module = compiler.CompileGlslToSpv(source, Utils::GLShaderStageToShaderC(stage), m_Filepath.c_str(), options);
				if (module.GetCompilationStatus() != shaderc_compilation_status_success) {
					KBR_CORE_ERROR("Vulkan Shader: GLSL to SPIR-V compilation failed for {0} ({1}):\n{2}", m_Filepath, Utils::GLShaderStageToString(stage), module.GetErrorMessage());
					KBR_CORE_ASSERT(false, "Vulkan Shader: GLSL to SPIR-V compilation failed");
				}

				m_VulkanSPIRV[stage] = std::vector<uint32_t>(module.cbegin(), module.cend());

				std::ofstream out(cachedPath, std::ios::out | std::ios::binary);
				if (out.is_open()) {
					auto& data = m_VulkanSPIRV[stage];
					out.write(reinterpret_cast<char*>(data.data()), data.size() * sizeof(uint32_t));
					out.flush();
					out.close();
					KBR_CORE_TRACE("Vulkan Shader: Compiled SPIR-V and cached to: {0}", cachedPath.string());
				}
				else {
					KBR_CORE_ERROR("Vulkan Shader: Failed to open cache file for writing: {0}", cachedPath.string());
				}
			}
		}
	}

	void VulkanShader::CreateShaderModules()
	{
		KBR_PROFILE_FUNCTION();

		const auto device = VulkanContext::Get().GetDevice();

		KBR_CORE_ASSERT(device, "VkDevice is null");

		m_ShaderModules.clear();
		m_PipelineShaderStageCreateInfos.clear();

		for (auto const& [glStage, spirvCode] : m_VulkanSPIRV)
		{
			if (spirvCode.empty())
			{
				KBR_CORE_ERROR("Vulkan Shader: No SPIR-V code found for stage {0} in shader {1}", Utils::GLShaderStageToString(glStage), m_Filepath);
				continue;
			}

			VkShaderModuleCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
			createInfo.codeSize = spirvCode.size() * sizeof(uint32_t);
			createInfo.pCode = spirvCode.data();

			VkShaderModule shaderModule;
			if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
			{
				KBR_CORE_ERROR("Vulkan Shader: Failed to create shader module for stage {0} in shader {1}", Utils::GLShaderStageToString(glStage), m_Filepath);
				continue;
			}

			VkShaderStageFlagBits vkStage = Utils::GLShaderStageToVulkanStage(glStage);
			m_ShaderModules[vkStage] = shaderModule;

			VkPipelineShaderStageCreateInfo shaderStageInfo{};
			shaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			shaderStageInfo.stage = vkStage;
			shaderStageInfo.module = shaderModule;
			shaderStageInfo.pName = "main";

			m_PipelineShaderStageCreateInfos.push_back(shaderStageInfo);
			KBR_CORE_TRACE("Vulkan Shader: Created VkShaderModule for stage {0} ({1})", m_Name, Utils::GLShaderStageToString(glStage));
		}
	}

	void VulkanShader::ReflectAllStages()
	{
		KBR_PROFILE_FUNCTION();

		KBR_CORE_TRACE("Shader reflection for {0}", m_Filepath.empty() ? m_Name : m_Filepath);

		for (auto const & [stage, spirvCode] : m_VulkanSPIRV)
		{
			if (!spirvCode.empty())
			{
				Reflect(Utils::GLShaderStageToVulkanStage(stage), spirvCode);
			}
		}
	}

	void VulkanShader::Reflect(const VkShaderStageFlagBits stage, const std::vector<uint32_t>& spirvCode)
	{
		const spirv_cross::Compiler compiler(spirvCode);
		spirv_cross::ShaderResources resources = compiler.get_shader_resources();

		const char* stageName = nullptr;
		switch (stage)
		{
		case VK_SHADER_STAGE_VERTEX_BIT:   stageName = "Vertex"; break;
		case VK_SHADER_STAGE_FRAGMENT_BIT: stageName = "Fragment"; break;
		case VK_SHADER_STAGE_GEOMETRY_BIT: stageName = "Geometry"; break;
		}

		KBR_CORE_TRACE("  Stage: {0}", stageName);
		KBR_CORE_TRACE("    Uniform Buffers: {0}", resources.uniform_buffers.size());
		for (const auto& resource : resources.uniform_buffers) {
			const auto& bufferType = compiler.get_type(resource.base_type_id);
			uint32_t set = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
			uint32_t binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
			size_t bufferSize = compiler.get_declared_struct_size(bufferType);
			KBR_CORE_TRACE("      Name: {0}, Set: {1}, Binding: {2}, Size: {3}", resource.name, set, binding, bufferSize);
			// Store this info (set, binding, type, stageFlags) for VkDescriptorSetLayout creation
		}

		KBR_CORE_TRACE("    Sampled Images (Textures/Samplers): {0}", resources.sampled_images.size());
		for (const auto& resource : resources.sampled_images) {
			uint32_t set = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
			uint32_t binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
			KBR_CORE_TRACE("      Name: {0}, Set: {1}, Binding: {2}", resource.name, set, binding);
			// Store this info for VkDescriptorSetLayout creation
		}

		KBR_CORE_TRACE("    Storage Buffers: {0}", resources.storage_buffers.size());
		for (const auto& resource : resources.storage_buffers) {
			const auto& bufferType = compiler.get_type(resource.base_type_id);
			uint32_t set = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
			uint32_t binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
			size_t bufferSize = compiler.get_declared_struct_size(bufferType);
			KBR_CORE_TRACE("      Name: {0}, Set: {1}, Binding: {2}, Approx Size: {3}", resource.name, set, binding, bufferSize);
		}

		KBR_CORE_TRACE("    Storage Images: {0}", resources.storage_images.size());
		for (const auto& resource : resources.storage_images) {
			uint32_t set = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
			uint32_t binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
			KBR_CORE_TRACE("      Name: {0}, Set: {1}, Binding: {2}", resource.name, set, binding);
		}

		KBR_CORE_TRACE("    Push Constant Buffers: {0}", resources.push_constant_buffers.size());
		for (const auto& resource : resources.push_constant_buffers) {
			const auto& bufferType = compiler.get_type(resource.base_type_id);
			uint32_t offset = 0; // Typically
			// SPIRV-Cross might need a bit more work to get exact offset for members if it's a struct.
		   // For a single push constant block, its range covers the whole block.
			size_t size = compiler.get_declared_struct_size(bufferType);

			// Get shader stage for this push constant more accurately
			// auto ranges = compiler.get_active_buffer_ranges(resource.id);
			// For now, we assume the 'stage' passed to Reflect applies.

			KBR_CORE_TRACE("      Name: {0}, Offset: {1}, Size: {2}", resource.name, offset, size);
			// Store this info (stageFlags, offset, size) for VkPushConstantRange creation
		}
	}
}
