#include "kbrpch.h"
#include "VulkanShader.h"

#include <glslang/Public/ResourceLimits.h>
#include <glslang/SPIRV/GlslangToSpv.h>

namespace Kerberos
{
	VulkanShader::VulkanShader(const std::string& filepath)
	{
		
	}

	VulkanShader::VulkanShader(std::string name, const std::string& vertexSrc, const std::string& fragmentSrc,
		const std::string& geometrySrc)
			: m_Name(std::move(name))
	{
		
	}

	VulkanShader::~VulkanShader()
	{
		
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

    std::vector<uint32_t> VulkanShader::CompileGLSLToSPIRV(const std::string& glslSource, EShLanguage shaderType)
    {
        KBR_PROFILE_FUNCTION();

        // 1. Create a shader object
        glslang::TShader shader(shaderType);

        // 2. Provide the shader source
        const char* shaderStrings[1] = { glslSource.c_str() };
        shader.setStrings(shaderStrings, 1);

        // 3. Set shader language version and profile
        shader.setEnvInput(glslang::EShSourceGlsl, shaderType,
            glslang::EShClientVulkan, 100); // Vulkan 1.1, GLSL 4.5
        shader.setEnvClient(glslang::EShClientVulkan, glslang::EShTargetVulkan_1_1);
        shader.setEnvTarget(glslang::EShTargetSpv, glslang::EShTargetSpv_1_3);

		const auto builtInResources = InitResources();

        // 4. Parse the shader
        constexpr auto messages = static_cast<EShMessages>(EShMsgDefault | EShMsgVulkanRules |
	        EShMsgSpvRules);
        if (!shader.parse(&builtInResources, 450, false, messages))
        {
            std::cerr << "GLSL parsing failed:\n" << shader.getInfoLog() << "\n"
                << shader.getInfoDebugLog() << '\n';
            throw std::runtime_error("GLSL parsing failed");
        }

        // 5. Create a program object and link the shader
        glslang::TProgram program;
        program.addShader(&shader);

        if (!program.link(messages))
        {
            std::cerr << "GLSL linking failed:\n" << program.getInfoLog() << "\n"
                << program.getInfoDebugLog() << '\n';
            throw std::runtime_error("GLSL linking failed");
        }

        // 6. Translate to SPIR-V
        std::vector<uint32_t> spirv;
        glslang::SpvOptions spvOptions;
        glslang::GlslangToSpv(*program.getIntermediate(shaderType), spirv,
            &spvOptions);

        return spirv;
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

	TBuiltInResource VulkanShader::InitResources()
    {
        TBuiltInResource resources;

        resources.maxLights = 32;
        resources.maxClipPlanes = 6;
        resources.maxTextureUnits = 32;
        resources.maxTextureCoords = 32;
        resources.maxVertexAttribs = 64;
        resources.maxVertexUniformComponents = 4096;
        resources.maxVaryingFloats = 64;
        resources.maxVertexTextureImageUnits = 32;
        resources.maxCombinedTextureImageUnits = 80;
        resources.maxTextureImageUnits = 32;
        resources.maxFragmentUniformComponents = 4096;
        resources.maxDrawBuffers = 32;
        resources.maxVertexUniformVectors = 128;
        resources.maxVaryingVectors = 8;
        resources.maxFragmentUniformVectors = 16;
        resources.maxVertexOutputVectors = 16;
        resources.maxFragmentInputVectors = 15;
        resources.minProgramTexelOffset = -8;
        resources.maxProgramTexelOffset = 7;
        resources.maxClipDistances = 8;
        resources.maxComputeWorkGroupCountX = 65535;
        resources.maxComputeWorkGroupCountY = 65535;
        resources.maxComputeWorkGroupCountZ = 65535;
        resources.maxComputeWorkGroupSizeX = 1024;
        resources.maxComputeWorkGroupSizeY = 1024;
        resources.maxComputeWorkGroupSizeZ = 64;
        resources.maxComputeUniformComponents = 1024;
        resources.maxComputeTextureImageUnits = 16;
        resources.maxComputeImageUniforms = 8;
        resources.maxComputeAtomicCounters = 8;
        resources.maxComputeAtomicCounterBuffers = 1;
        resources.maxVaryingComponents = 60;
        resources.maxVertexOutputComponents = 64;
        resources.maxGeometryInputComponents = 64;
        resources.maxGeometryOutputComponents = 128;
        resources.maxFragmentInputComponents = 128;
        resources.maxImageUnits = 8;
        resources.maxCombinedImageUnitsAndFragmentOutputs = 8;
        resources.maxCombinedShaderOutputResources = 8;
        resources.maxImageSamples = 0;
        resources.maxVertexImageUniforms = 0;
        resources.maxTessControlImageUniforms = 0;
        resources.maxTessEvaluationImageUniforms = 0;
        resources.maxGeometryImageUniforms = 0;
        resources.maxFragmentImageUniforms = 8;
        resources.maxCombinedImageUniforms = 8;
        resources.maxGeometryTextureImageUnits = 16;
        resources.maxGeometryOutputVertices = 256;
        resources.maxGeometryTotalOutputComponents = 1024;
        resources.maxGeometryUniformComponents = 1024;
        resources.maxGeometryVaryingComponents = 64;
        resources.maxTessControlInputComponents = 128;
        resources.maxTessControlOutputComponents = 128;
        resources.maxTessControlTextureImageUnits = 16;
        resources.maxTessControlUniformComponents = 1024;
        resources.maxTessControlTotalOutputComponents = 4096;
        resources.maxTessEvaluationInputComponents = 128;
        resources.maxTessEvaluationOutputComponents = 128;
        resources.maxTessEvaluationTextureImageUnits = 16;
        resources.maxTessEvaluationUniformComponents = 1024;
        resources.maxTessPatchComponents = 120;
        resources.maxPatchVertices = 32;
        resources.maxTessGenLevel = 64;
        resources.maxViewports = 16;
        resources.maxVertexAtomicCounters = 0;
        resources.maxTessControlAtomicCounters = 0;
        resources.maxTessEvaluationAtomicCounters = 0;
        resources.maxGeometryAtomicCounters = 0;
        resources.maxFragmentAtomicCounters = 8;
        resources.maxCombinedAtomicCounters = 8;
        resources.maxAtomicCounterBindings = 1;
        resources.maxVertexAtomicCounterBuffers = 0;
        resources.maxTessControlAtomicCounterBuffers = 0;
        resources.maxTessEvaluationAtomicCounterBuffers = 0;
        resources.maxGeometryAtomicCounterBuffers = 0;
        resources.maxFragmentAtomicCounterBuffers = 1;
        resources.maxCombinedAtomicCounterBuffers = 1;
        resources.maxAtomicCounterBufferSize = 16384;
        resources.maxTransformFeedbackBuffers = 4;
        resources.maxTransformFeedbackInterleavedComponents = 64;
        resources.maxCullDistances = 8;
        resources.maxCombinedClipAndCullDistances = 8;
        resources.maxSamples = 4;
        resources.maxMeshOutputVerticesNV = 256;
        resources.maxMeshOutputPrimitivesNV = 512;
        resources.maxMeshWorkGroupSizeX_NV = 32;
        resources.maxMeshWorkGroupSizeY_NV = 1;
        resources.maxMeshWorkGroupSizeZ_NV = 1;
        resources.maxTaskWorkGroupSizeX_NV = 32;
        resources.maxTaskWorkGroupSizeY_NV = 1;
        resources.maxTaskWorkGroupSizeZ_NV = 1;
        resources.maxMeshViewCountNV = 4;

        resources.limits.nonInductiveForLoops = 1;
        resources.limits.whileLoops = 1;
        resources.limits.doWhileLoops = 1;
        resources.limits.generalUniformIndexing = 1;
        resources.limits.generalAttributeMatrixVectorIndexing = 1;
        resources.limits.generalVaryingIndexing = 1;
        resources.limits.generalSamplerIndexing = 1;
        resources.limits.generalVariableIndexing = 1;
        resources.limits.generalConstantMatrixVectorIndexing = 1;

        return resources;
    }
}
