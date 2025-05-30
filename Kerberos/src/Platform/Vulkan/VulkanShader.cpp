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
		auto shaderSources = SplitShaderSource(source);

		{
			// TODO: Time shader compilation and caching
			CompileOrGetVulkanBinaries(shaderSources);
			CreateShaderModule();
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
		CreateShaderModule();
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

	std::pair<std::string, std::string> VulkanShader::SplitShaderSource(const std::string& source)
	{
		KBR_PROFILE_FUNCTION();

		const size_t vertexPos = source.find("#type vertex");
		const size_t fragmentPos = source.find("#type fragment");

		if (vertexPos == std::string::npos || fragmentPos == std::string::npos)
		{
			throw std::runtime_error(
				"Shader file must contain '#type vertex' and '#type fragment' "
				"directives.");
		}

		std::string vertexSource = source.substr(vertexPos + std::string("#type vertex").length(),
			fragmentPos - vertexPos -
			std::string("#type vertex").length());
		std::string fragmentSource = source.substr(fragmentPos +
			std::string("#type fragment").length());

		return { vertexSource, fragmentSource };
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
			std::filesystem::path shaderFilePath = m_Filepath; // Used for unique cache naming
			// Ensure filename is valid for filesystem operations
			std::string baseFilename = shaderFilePath.empty() ? m_Name : shaderFilePath.filename().string();
			if (baseFilename.empty()) baseFilename = "unnamed_shader"; // Fallback

			std::filesystem::path cachedPath = cacheDirectory / (baseFilename + Utils::ShaderStageCachedFileExtension(stage));

			std::ifstream in(cachedPath, std::ios::in | std::ios::binary);
			if (in.is_open()) {
				in.seekg(0, std::ios::end);
				auto size = in.tellg();
				in.seekg(0, std::ios::beg);

				auto& data = m_VulkanSPIRV[stage];
				data.resize(size / sizeof(uint32_t));
				in.read((char*)data.data(), size);
				in.close();
				KBR_CORE_TRACE("Vulkan Shader: Loaded SPIR-V from cache: {0}", cachedPath.string());
			}
			else {
				shaderc::SpvCompilationResult module = compiler.CompileGlslToSpv(source, Utils::GLShaderStageToShaderC(stage), m_Filepath.c_str(), options);
				if (module.GetCompilationStatus() != shaderc_compilation_status_success) {
					KBR_CORE_ERROR("Vulkan Shader: GLSL to SPIR-V compilation failed for {0} ({1}):\n{2}",
						m_Filepath, Utils::GLShaderStageToString(stage), module.GetErrorMessage());
					KBR_CORE_ASSERT(false);
				}

				m_VulkanSPIRV[stage] = std::vector<uint32_t>(module.cbegin(), module.cend());

				std::ofstream out(cachedPath, std::ios::out | std::ios::binary);
				if (out.is_open()) {
					auto& data = m_VulkanSPIRV[stage];
					out.write((char*)data.data(), data.size() * sizeof(uint32_t));
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

	VkShaderModule VulkanShader::CreateShaderModule(const VkDevice device, const std::vector<uint32_t>& spirvCode)
	{
		KBR_PROFILE_FUNCTION();

		VkShaderModuleCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = spirvCode.size() * sizeof(uint32_t);
		createInfo.pCode = spirvCode.data();

		VkShaderModule shaderModule;
		if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) !=
			VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create shader module!");
		}

		return shaderModule;
	}

	//TBuiltInResource VulkanShader::InitResources()
 //   {
 //       TBuiltInResource resources;

 //       resources.maxLights = 32;
 //       resources.maxClipPlanes = 6;
 //       resources.maxTextureUnits = 32;
 //       resources.maxTextureCoords = 32;
 //       resources.maxVertexAttribs = 64;
 //       resources.maxVertexUniformComponents = 4096;
 //       resources.maxVaryingFloats = 64;
 //       resources.maxVertexTextureImageUnits = 32;
 //       resources.maxCombinedTextureImageUnits = 80;
 //       resources.maxTextureImageUnits = 32;
 //       resources.maxFragmentUniformComponents = 4096;
 //       resources.maxDrawBuffers = 32;
 //       resources.maxVertexUniformVectors = 128;
 //       resources.maxVaryingVectors = 8;
 //       resources.maxFragmentUniformVectors = 16;
 //       resources.maxVertexOutputVectors = 16;
 //       resources.maxFragmentInputVectors = 15;
 //       resources.minProgramTexelOffset = -8;
 //       resources.maxProgramTexelOffset = 7;
 //       resources.maxClipDistances = 8;
 //       resources.maxComputeWorkGroupCountX = 65535;
 //       resources.maxComputeWorkGroupCountY = 65535;
 //       resources.maxComputeWorkGroupCountZ = 65535;
 //       resources.maxComputeWorkGroupSizeX = 1024;
 //       resources.maxComputeWorkGroupSizeY = 1024;
 //       resources.maxComputeWorkGroupSizeZ = 64;
 //       resources.maxComputeUniformComponents = 1024;
 //       resources.maxComputeTextureImageUnits = 16;
 //       resources.maxComputeImageUniforms = 8;
 //       resources.maxComputeAtomicCounters = 8;
 //       resources.maxComputeAtomicCounterBuffers = 1;
 //       resources.maxVaryingComponents = 60;
 //       resources.maxVertexOutputComponents = 64;
 //       resources.maxGeometryInputComponents = 64;
 //       resources.maxGeometryOutputComponents = 128;
 //       resources.maxFragmentInputComponents = 128;
 //       resources.maxImageUnits = 8;
 //       resources.maxCombinedImageUnitsAndFragmentOutputs = 8;
 //       resources.maxCombinedShaderOutputResources = 8;
 //       resources.maxImageSamples = 0;
 //       resources.maxVertexImageUniforms = 0;
 //       resources.maxTessControlImageUniforms = 0;
 //       resources.maxTessEvaluationImageUniforms = 0;
 //       resources.maxGeometryImageUniforms = 0;
 //       resources.maxFragmentImageUniforms = 8;
 //       resources.maxCombinedImageUniforms = 8;
 //       resources.maxGeometryTextureImageUnits = 16;
 //       resources.maxGeometryOutputVertices = 256;
 //       resources.maxGeometryTotalOutputComponents = 1024;
 //       resources.maxGeometryUniformComponents = 1024;
 //       resources.maxGeometryVaryingComponents = 64;
 //       resources.maxTessControlInputComponents = 128;
 //       resources.maxTessControlOutputComponents = 128;
 //       resources.maxTessControlTextureImageUnits = 16;
 //       resources.maxTessControlUniformComponents = 1024;
 //       resources.maxTessControlTotalOutputComponents = 4096;
 //       resources.maxTessEvaluationInputComponents = 128;
 //       resources.maxTessEvaluationOutputComponents = 128;
 //       resources.maxTessEvaluationTextureImageUnits = 16;
 //       resources.maxTessEvaluationUniformComponents = 1024;
 //       resources.maxTessPatchComponents = 120;
 //       resources.maxPatchVertices = 32;
 //       resources.maxTessGenLevel = 64;
 //       resources.maxViewports = 16;
 //       resources.maxVertexAtomicCounters = 0;
 //       resources.maxTessControlAtomicCounters = 0;
 //       resources.maxTessEvaluationAtomicCounters = 0;
 //       resources.maxGeometryAtomicCounters = 0;
 //       resources.maxFragmentAtomicCounters = 8;
 //       resources.maxCombinedAtomicCounters = 8;
 //       resources.maxAtomicCounterBindings = 1;
 //       resources.maxVertexAtomicCounterBuffers = 0;
 //       resources.maxTessControlAtomicCounterBuffers = 0;
 //       resources.maxTessEvaluationAtomicCounterBuffers = 0;
 //       resources.maxGeometryAtomicCounterBuffers = 0;
 //       resources.maxFragmentAtomicCounterBuffers = 1;
 //       resources.maxCombinedAtomicCounterBuffers = 1;
 //       resources.maxAtomicCounterBufferSize = 16384;
 //       resources.maxTransformFeedbackBuffers = 4;
 //       resources.maxTransformFeedbackInterleavedComponents = 64;
 //       resources.maxCullDistances = 8;
 //       resources.maxCombinedClipAndCullDistances = 8;
 //       resources.maxSamples = 4;
 //       resources.maxMeshOutputVerticesNV = 256;
 //       resources.maxMeshOutputPrimitivesNV = 512;
 //       resources.maxMeshWorkGroupSizeX_NV = 32;
 //       resources.maxMeshWorkGroupSizeY_NV = 1;
 //       resources.maxMeshWorkGroupSizeZ_NV = 1;
 //       resources.maxTaskWorkGroupSizeX_NV = 32;
 //       resources.maxTaskWorkGroupSizeY_NV = 1;
 //       resources.maxTaskWorkGroupSizeZ_NV = 1;
 //       resources.maxMeshViewCountNV = 4;

 //       resources.limits.nonInductiveForLoops = 1;
 //       resources.limits.whileLoops = 1;
 //       resources.limits.doWhileLoops = 1;
 //       resources.limits.generalUniformIndexing = 1;
 //       resources.limits.generalAttributeMatrixVectorIndexing = 1;
 //       resources.limits.generalVaryingIndexing = 1;
 //       resources.limits.generalSamplerIndexing = 1;
 //       resources.limits.generalVariableIndexing = 1;
 //       resources.limits.generalConstantMatrixVectorIndexing = 1;

 //       return resources;
 //   }
}
