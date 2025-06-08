#include "kbrpch.h"
#include "VulkanShader.h"

#include <filesystem>
#include <ranges>

#include <shaderc/shaderc.hpp>
#include <spirv_cross/spirv_cross.hpp>
#include <spirv_cross/spirv_glsl.hpp>

#include "VulkanContext.h"
#include "Kerberos/Core/Timer.h"

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

		static GLenum ShaderTypeFromString(const std::string& type)
		{
			if (type == "vertex")
				return GL_VERTEX_SHADER;
			if (type == "fragment" || type == "pixel")
				return GL_FRAGMENT_SHADER;
			if (type == "geometry")
				return GL_GEOMETRY_SHADER;
			KBR_CORE_ASSERT(false, "Unknown shader type!")
				return 0;
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

		static VkFormat GetVulkanFormat(const spirv_cross::SPIRType& type) {
			if (type.basetype == spirv_cross::SPIRType::Float) {
				if (type.vecsize == 1 && type.width == 32) return VK_FORMAT_R32_SFLOAT;
				if (type.vecsize == 2 && type.width == 32) return VK_FORMAT_R32G32_SFLOAT;
				if (type.vecsize == 3 && type.width == 32) return VK_FORMAT_R32G32B32_SFLOAT;
				if (type.vecsize == 4 && type.width == 32) return VK_FORMAT_R32G32B32A32_SFLOAT;
			}
			if (type.basetype == spirv_cross::SPIRType::Int) {
				if (type.vecsize == 1 && type.width == 32) return VK_FORMAT_R32_SINT;
				if (type.vecsize == 2 && type.width == 32) return VK_FORMAT_R32G32_SINT;
				if (type.vecsize == 3 && type.width == 32) return VK_FORMAT_R32G32B32_SINT;
				if (type.vecsize == 4 && type.width == 32) return VK_FORMAT_R32G32B32A32_SINT;
			}
			if (type.basetype == spirv_cross::SPIRType::UInt) {
				if (type.vecsize == 1 && type.width == 32) return VK_FORMAT_R32_UINT;
				if (type.vecsize == 2 && type.width == 32) return VK_FORMAT_R32G32_UINT;
				if (type.vecsize == 3 && type.width == 32) return VK_FORMAT_R32G32B32_UINT;
				if (type.vecsize == 4 && type.width == 32) return VK_FORMAT_R32G32B32A32_UINT;
			}

			KBR_CORE_ERROR("Failed to determine VkFormat for SPIR-V type. BaseType: {0}, VecSize: {1}, Width: {2}",
				static_cast<int>(type.basetype), type.vecsize, type.width);
			return VK_FORMAT_UNDEFINED;
		}

		static uint32_t GetFormatSize(const VkFormat format) {
			switch (format) {
			case VK_FORMAT_R32_SFLOAT:          return 4;
			case VK_FORMAT_R32G32_SFLOAT:       return 8;
			case VK_FORMAT_R32G32B32_SFLOAT:    return 12;
			case VK_FORMAT_R32G32B32A32_SFLOAT: return 16;
			case VK_FORMAT_R32_SINT:            return 4;
			case VK_FORMAT_R32G32_SINT:         return 8;
			case VK_FORMAT_R32G32B32_SINT:      return 12;
			case VK_FORMAT_R32G32B32A32_SINT:   return 16;
			case VK_FORMAT_R32_UINT:            return 4;
			case VK_FORMAT_R32G32_UINT:         return 8;
			case VK_FORMAT_R32G32B32_UINT:      return 12;
			case VK_FORMAT_R32G32B32A32_UINT:   return 16;

			default: KBR_CORE_ERROR("Unknown VkFormat for size calculation: {0}", static_cast<int>(format)); return 0;
			}
		}
	}

	VulkanShader::VulkanShader(const std::string& filepath)
		: m_Filepath(filepath)
	{
		KBR_PROFILE_FUNCTION();

		/// I haven't converted the other shaders to be Vulkan compatible yet,
		/// and i don't want to modify the source code of the Editor
		/*if (filepath != "assets/shaders/shader3d-vulkan.glsl" && filepath != "assets/shaders/shader3d-basic-vulkan.glsl")
			return;*/

		Utils::CreateCacheDirectoryIfNeeded();

		const std::string source = ReadShaderFile(filepath);
		const auto shaderSources = SplitShaderSource(source);

		{
			struct ProfileResult
			{
				const char* Name;
				float Time;
			};

			Timer timer("VulkanShader - Shader compilation", [&](const ProfileResult& res)
			{
					KBR_CORE_TRACE("Shader compilation took {0} ms", res.Time);
			});

			CompileOrGetVulkanBinaries(shaderSources);
			CreateShaderModules();
			ReflectAllStages();
			CreateDescriptorSetLayouts();
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
		if (!geometrySrc.empty()) {
			sources[GL_GEOMETRY_SHADER] = geometrySrc;
		}

		CompileOrGetVulkanBinaries(sources);
		CreateShaderModules();
		ReflectAllStages();
		CreateDescriptorSetLayouts();
	}

	VulkanShader::~VulkanShader()
	{
		KBR_PROFILE_FUNCTION();

		const VkDevice device = VulkanContext::Get().GetDevice();
		KBR_CORE_ASSERT(device, "Vulkan logical device is null in destructor!");

		for (const auto& shaderModule : m_ShaderModules | std::views::values) {
			if (shaderModule != VK_NULL_HANDLE) {
				vkDestroyShaderModule(device, shaderModule, nullptr);
			}
		}
		m_ShaderModules.clear();
		m_PipelineShaderStageCreateInfos.clear();

		for (const VkDescriptorSetLayout layout : m_DescriptorSetLayouts) {
			if (layout != VK_NULL_HANDLE) {
				vkDestroyDescriptorSetLayout(device, layout, nullptr);
			}
		}
		m_DescriptorSetLayouts.clear();

		m_VertexInputAttributeDescriptions.clear();
		m_VertexInputBindingDescriptions.clear();
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

		std::unordered_map<GLenum, std::string> shaderSources;

		const char* typeToken = "#type";
		const size_t typeTokenLength = strlen(typeToken);
		size_t pos = source.find(typeToken, 0);

		while (pos != std::string::npos)
		{
			const size_t eol = source.find_first_of("\r\n", pos);
			KBR_CORE_ASSERT(eol != std::string::npos, "Syntax error!");
			const size_t begin = pos + typeTokenLength + 1;
			std::string type = source.substr(begin, eol - begin);
			KBR_CORE_ASSERT(type == "vertex" || type == "fragment" || type == "pixel" || type == "geometry", "Invalid shader type specified!");

			const size_t nextLinePos = source.find_first_not_of("\r\n", eol);
			pos = source.find(typeToken, nextLinePos);

			shaderSources[Utils::ShaderTypeFromString(type)]
				= source.substr(nextLinePos, pos - (nextLinePos == std::string::npos ? source.size() - 1 : nextLinePos));
		}

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

			m_PipelineShaderStageCreateInfos[vkStage] = shaderStageInfo;
			KBR_CORE_TRACE("Vulkan Shader: Created VkShaderModule for stage {0} ({1})", m_Name, Utils::GLShaderStageToString(glStage));
		}
	}

	void VulkanShader::ReflectAllStages()
	{
		KBR_PROFILE_FUNCTION();

		KBR_CORE_TRACE("Shader reflection for {0}", m_Filepath.empty() ? m_Name : m_Filepath);

		for (auto const& [stage, spirvCode] : m_VulkanSPIRV)
		{
			if (!spirvCode.empty())
			{
				Reflect(Utils::GLShaderStageToVulkanStage(stage), spirvCode);
			}
		}
	}

	void VulkanShader::CreateDescriptorSetLayouts()
	{
		KBR_PROFILE_FUNCTION();

		const VkDevice device = VulkanContext::Get().GetDevice();
		KBR_CORE_ASSERT(device, "Vulkan logical device is null in CreateDescriptorSetLayouts!");

		/// Clear existing descriptor set layouts
		for (const VkDescriptorSetLayout layout : m_DescriptorSetLayouts)
		{
			if (layout != VK_NULL_HANDLE)
			{
				vkDestroyDescriptorSetLayout(device, layout, nullptr);
			}
		}
		m_DescriptorSetLayouts.clear();

		if (!m_DescriptorSetLayoutsInfo.empty())
		{
			const uint32_t maxSet = m_DescriptorSetLayoutsInfo.rbegin()->first;
			m_DescriptorSetLayouts.resize(maxSet + 1, VK_NULL_HANDLE);
		}

		for (auto const& [setIndex, bindingsMap] : m_DescriptorSetLayoutsInfo)
		{
			std::vector<VkDescriptorSetLayoutBinding> bindings;
			for (const auto& bindingInfo : bindingsMap | std::views::values)
			{
				bindings.push_back(bindingInfo);
			}

			if (bindings.empty())
			{
				KBR_CORE_WARN("Vulkan Shader: No bindings found for descriptor set {0} in shader {1}", setIndex, m_Filepath);
				continue;
			}

			VkDescriptorSetLayoutCreateInfo layoutCreateInfo{};
			layoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			layoutCreateInfo.bindingCount = static_cast<uint32_t>(bindings.size());
			layoutCreateInfo.pBindings = bindings.data();

			VkDescriptorSetLayout descriptorSetLayout;
			if (vkCreateDescriptorSetLayout(device, &layoutCreateInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS)
			{
				KBR_CORE_ERROR("Vulkan Shader: Failed to create descriptor set layout for set {0} in shader {1}", setIndex, m_Filepath);
				continue;
			}
			KBR_CORE_TRACE("Vulkan Shader: Created descriptor set layout for set {0} in shader {1}", setIndex, m_Filepath);
			m_DescriptorSetLayouts[setIndex] = descriptorSetLayout;
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

			auto& bindingsForSet = m_DescriptorSetLayoutsInfo[set];
			if (!bindingsForSet.contains(binding))
			{
				constexpr uint32_t descriptorCount = 1;
				/// Create a new binding for this set
				VkDescriptorSetLayoutBinding newBinding{};
				newBinding.binding = binding;
				newBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				newBinding.descriptorCount = descriptorCount;
				newBinding.stageFlags = stage; // Initial stage
				newBinding.pImmutableSamplers = nullptr;

				bindingsForSet[binding] = newBinding;
			}
			else
			{
				/// Binding already exists, merge stage flags
				bindingsForSet[binding].stageFlags |= stage;
			}
		}

		KBR_CORE_TRACE("    Sampled Images (Textures/Samplers): {0}", resources.sampled_images.size());
		for (const auto& resource : resources.sampled_images) {
			uint32_t set = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
			uint32_t binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
			uint32_t descriptorCount = 1;

			// Check if it's an array of textures (e.g., `sampler2D textures[4]`)
			const spirv_cross::SPIRType& type = compiler.get_type(resource.base_type_id);
			if (!type.array.empty()) {
				descriptorCount = type.array[0]; // Assuming 1D array for simplicity
				if (descriptorCount == 0) // Unsized array (e.g., `sampler2D textures[]`)
					//descriptorCount = VulkanContext::Get().GetCapabilities().maxSamplerAllocationCount; // Or some max you define
					descriptorCount = 1; // Default to 1 if unsized
			}

			KBR_CORE_TRACE("      Name: {0}, Set: {1}, Binding: {2}, Count: {3}", resource.name, set, binding, descriptorCount);

			auto& bindingsForSet = m_DescriptorSetLayoutsInfo[set];
			if (!bindingsForSet.contains(binding))
			{
				VkDescriptorSetLayoutBinding newBinding{};
				newBinding.binding = binding;
				newBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				newBinding.descriptorCount = descriptorCount;
				newBinding.stageFlags = stage;
				newBinding.pImmutableSamplers = nullptr;

				bindingsForSet[binding] = newBinding;
			}
			else
			{
				bindingsForSet[binding].stageFlags |= stage;
				// KBR_CORE_ASSERT(bindingsForSet[binding].descriptorCount == descriptorCount, "Descriptor count mismatch for binding!");
				// You might want to handle this warning/error if a binding count differs between stages.
			}
		}

		KBR_CORE_TRACE("    Storage Buffers: {0}", resources.storage_buffers.size());
		for (const auto& resource : resources.storage_buffers) {
			const auto& bufferType = compiler.get_type(resource.base_type_id);
			uint32_t set = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
			uint32_t binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
			size_t bufferSize = compiler.get_declared_struct_size(bufferType);

			KBR_CORE_TRACE("      Name: {0}, Set: {1}, Binding: {2}, Approx Size: {3}", resource.name, set, binding, bufferSize);

			auto& bindingsForSet = m_DescriptorSetLayoutsInfo[set];
			if (!bindingsForSet.contains(binding))
			{
				constexpr uint32_t descriptorCount = 1;
				VkDescriptorSetLayoutBinding newBinding{};
				newBinding.binding = binding;
				newBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
				newBinding.descriptorCount = descriptorCount;
				newBinding.stageFlags = stage;
				newBinding.pImmutableSamplers = nullptr;

				bindingsForSet[binding] = newBinding;
			}
			else
			{
				bindingsForSet[binding].stageFlags |= stage;
			}
		}

		KBR_CORE_TRACE("    Storage Images: {0}", resources.storage_images.size());
		for (const auto& resource : resources.storage_images) {
			uint32_t set = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
			uint32_t binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
			uint32_t descriptorCount = 1;
			const spirv_cross::SPIRType& type = compiler.get_type(resource.base_type_id);
			if (!type.array.empty()) {
				descriptorCount = type.array[0];
				if (descriptorCount == 0)
					//descriptorCount = VulkanContext::Get().GetCapabilities().maxStorageImageAllocationCount; // Or some max
					descriptorCount = 1; // Default to 1 if unsized
			}

			KBR_CORE_TRACE("      Name: {0}, Set: {1}, Binding: {2}", resource.name, set, binding);

			auto& bindingsForSet = m_DescriptorSetLayoutsInfo[set];
			if (!bindingsForSet.contains(binding))
			{
				VkDescriptorSetLayoutBinding newBinding{};
				newBinding.binding = binding;
				newBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
				newBinding.descriptorCount = descriptorCount;
				newBinding.stageFlags = stage;
				newBinding.pImmutableSamplers = nullptr;

				bindingsForSet[binding] = newBinding;
			}
			else
			{
				bindingsForSet[binding].stageFlags |= stage;
			}
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

			const auto ranges = compiler.get_active_buffer_ranges(resource.id);
			bool found = false;
			for (auto& existingRange : m_PushConstantRanges) {
				// Assuming a single push constant block starts at offset 0.
				// If you have multiple distinct push constant blocks with different offsets,
				// this comparison needs to be more robust (e.g., comparing resource name or actual offset from SPIR-V).
				// For a typical use case with one push constant block, offset will be 0.
				if (existingRange.offset == 0 && existingRange.size == size) {
					existingRange.stageFlags |= stage; // Merge stage flags
					found = true;
					//KBR_CORE_TRACE("      Merged Push Constant Range: Name: {0}}", resource.name);
					break;
				}
			}

			if (!found) {
				VkPushConstantRange pushConstantRange{};
				pushConstantRange.stageFlags = stage;
				pushConstantRange.offset = offset;
				pushConstantRange.size = static_cast<uint32_t>(size);
				m_PushConstantRanges.push_back(pushConstantRange);
			}
		}

		/// Vertex Input Attributes
		if (stage == VK_SHADER_STAGE_VERTEX_BIT)
		{
			KBR_CORE_TRACE("    Vertex Input Attributes: {0}", resources.stage_inputs.size());

			m_VertexInputAttributeDescriptions.clear();
			m_VertexInputBindingDescriptions.clear();

			/// Temporary struct to hold reflection data and sort it by location
			struct VertexAttributeTemp {
				spirv_cross::Resource resource;
				uint32_t location;
				VkFormat format;
				uint32_t size;
			};
			std::vector<VertexAttributeTemp> tempAttributes;

			for (const auto& resource : resources.stage_inputs) {
				const spirv_cross::SPIRType& type = compiler.get_type(resource.base_type_id);
				uint32_t location = compiler.get_decoration(resource.id, spv::DecorationLocation);
				VkFormat format = Utils::GetVulkanFormat(type);
				uint32_t size = Utils::GetFormatSize(format);

				if (format == VK_FORMAT_UNDEFINED || size == 0) {
					KBR_CORE_ERROR("Vulkan Shader: Failed to get format or size for vertex attribute '{0}' at location {1}", resource.name, location);
					continue;
				}

				tempAttributes.push_back({ resource, location, format, size });
			}

			/// Sort attributes by location to ensure correct offset calculation for interleaved data
			std::ranges::sort(tempAttributes,
			                  [](const VertexAttributeTemp& a, const VertexAttributeTemp& b) {
				                  return a.location < b.location;
			                  });

			uint32_t currentOffset = 0;
			uint32_t currentBinding = 0;
			uint32_t totalStride = 0;

			for (const auto& attr : tempAttributes)
			{
				VkVertexInputAttributeDescription attributeDesc{};
				attributeDesc.binding = currentBinding;
				attributeDesc.location = attr.location;
				attributeDesc.format = attr.format;
				attributeDesc.offset = currentOffset;

				m_VertexInputAttributeDescriptions.push_back(attributeDesc);

				currentOffset += attr.size;
				totalStride += attr.size;

				KBR_CORE_TRACE("      Input: {0}, Location: {1}, Format: {2}, Offset: {3}, Size: {4}", attr.resource.name, attr.location, static_cast<int>(attr.format), currentOffset, attr.size);
			}

			VkVertexInputBindingDescription bindingDesc{};
			bindingDesc.binding = currentBinding;
			bindingDesc.stride = totalStride;
			bindingDesc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

			m_VertexInputBindingDescriptions.push_back(bindingDesc);

			KBR_CORE_TRACE("    Vertex Binding Description: Binding: {0}, Stride: {1}, InputRate: {2}", bindingDesc.binding, bindingDesc.stride, static_cast<int>(bindingDesc.inputRate));
		}
	}
}
