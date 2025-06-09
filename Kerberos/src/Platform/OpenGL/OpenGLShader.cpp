#include "kbrpch.h"
#include "OpenGLShader.h"

#include "Kerberos/Core.h"

#include <fstream>
#include <filesystem>
#include <glm/gtc/type_ptr.hpp>
#include <glad/glad.h>
#include <shaderc/shaderc.h>
#include <shaderc/shaderc.hpp>
#include <spirv_cross/spirv_glsl.hpp>

#include "Kerberos/Core/Timer.h"


namespace Kerberos
{
	namespace Utils
	{
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

		static shaderc_shader_kind GLShaderStageToShaderC(const GLenum stage)
		{
			switch (stage)
			{
			case GL_VERTEX_SHADER:   return shaderc_glsl_vertex_shader;
			case GL_FRAGMENT_SHADER: return shaderc_glsl_fragment_shader;
			case GL_GEOMETRY_SHADER: return shaderc_glsl_geometry_shader;
			}
			KBR_CORE_ASSERT(false, "Not supported shader type");
			return static_cast<shaderc_shader_kind>(0);
		}

		static const char* GLShaderStageToString(const GLenum stage)
		{
			switch (stage)
			{
			case GL_VERTEX_SHADER:   return "GL_VERTEX_SHADER";
			case GL_FRAGMENT_SHADER: return "GL_FRAGMENT_SHADER";
			case GL_GEOMETRY_SHADER: return "GL_GEOMETRY_SHADER";
			}
			KBR_CORE_ASSERT(false, "Not supported shader type");
			return nullptr;
		}

		static const char* GetCacheDirectory()
		{
			return "assets/cache/shader/opengl";
		}

		static void CreateCacheDirectoryIfNeeded()
		{
			const std::string cacheDirectory = GetCacheDirectory();
			if (!std::filesystem::exists(cacheDirectory))
				std::filesystem::create_directories(cacheDirectory);
		}

		static const char* GLShaderStageCachedOpenGLFileExtension(const uint32_t stage)
		{
			switch (stage)
			{
			case GL_VERTEX_SHADER:    return ".cached_opengl.vert";
			case GL_FRAGMENT_SHADER:  return ".cached_opengl.frag";
			case GL_GEOMETRY_SHADER:  return ".cached_opengl.geom";
			default: 
				KBR_CORE_ASSERT(false, "Unknown shader stage for OpenGL cached file extension!");
				return "";
			}
		}

		static const char* GLShaderStageCachedVulkanFileExtension(const uint32_t stage)
		{
			switch (stage)
			{
			case GL_VERTEX_SHADER:    return ".cached_vulkan.vert";
			case GL_FRAGMENT_SHADER:  return ".cached_vulkan.frag";
			case GL_GEOMETRY_SHADER:  return ".cached_vulkan.geom";
				default: 
				KBR_CORE_ASSERT(false, "Unknown shader stage for Vulkan cached file extension!");
				return "";
			}
		}
	}
	

	OpenGLShader::OpenGLShader(const std::string& filepath)
		: m_FilePath(filepath)
	{
		KBR_PROFILE_FUNCTION();

		Utils::CreateCacheDirectoryIfNeeded();

		const std::string source = ReadFile(filepath);
		const auto shaderSources = Preprocess(source);
		//Compile(shaderSources);

		{
			struct ProfileResult
			{
				const char* Name;
				float Time;
			};

			Timer timer("OpenGLShader - Shader compilation", [&](const ProfileResult& res)
				{
					KBR_CORE_TRACE("Shader compilation took {0} ms", res.Time);
				});

			CompileOrGetVulkanBinaries(shaderSources);
			CompileOrGetOpenGLBinaries();
			CreateProgram();
		}

		const std::filesystem::path path = filepath;
		m_Name = path.stem().string();
	}

	OpenGLShader::OpenGLShader(std::string name, const std::string& vertexSrc, const std::string& fragmentSrc, const std::string& geometrySrc)
		: m_Name(std::move(name))
	{
		std::unordered_map<GLenum, std::string> sources;
		sources[GL_VERTEX_SHADER] = vertexSrc;
		sources[GL_FRAGMENT_SHADER] = fragmentSrc;
		if (!geometrySrc.empty())
			sources[GL_GEOMETRY_SHADER] = geometrySrc;

		CompileOrGetVulkanBinaries(sources);
		CompileOrGetOpenGLBinaries();
		CreateProgram();
		//Compile(sources);
	}

	OpenGLShader::~OpenGLShader()
	{
		glDeleteProgram(m_RendererID);
	}

	void OpenGLShader::Bind() const
	{
		glUseProgram(m_RendererID);
	}

	void OpenGLShader::Unbind() const
	{
		glUseProgram(0);
	}

	void OpenGLShader::SetInt(const std::string& name, const int value)
	{
		UploadUniformInt(name, value);
	}

	void OpenGLShader::SetIntArray(const std::string& name, int* values, const uint32_t count)
	{
		UploadUniformIntArray(name, values, count);
	}

	void OpenGLShader::SetFloat(const std::string& name, const float value)
	{
		UploadUniformFloat(name, value);
	}

	void OpenGLShader::SetFloat3(const std::string& name, const glm::vec3& value)
	{
		UploadUniformFloat3(name, value);
	}

	void OpenGLShader::SetFloat4(const std::string& name, const glm::vec4& value)
	{
		UploadUniformFloat4(name, value);
	}

	void OpenGLShader::SetMat4(const std::string& name, const glm::mat4& value)
	{
		UploadUniformMat4(name, value);
	}

	void OpenGLShader::SetMaterial(const std::string& name, const Ref<Material>& material)
	{
		UploadUniformFloat3(name + ".ambient", material->Ambient);
		UploadUniformFloat3(name + ".diffuse", material->Diffuse);
		UploadUniformFloat3(name + ".specular", material->Specular);
		UploadUniformFloat(name + ".shininess", material->Shininess);
	}

	void OpenGLShader::UploadUniformInt(const std::string& name, const int value) const
	{
		const GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniform1i(location, value);
	}

	void OpenGLShader::UploadUniformIntArray(const std::string& name, const int* values, const uint32_t count) const
	{
		const GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniform1iv(location, static_cast<int>(count), values);
	}

	void OpenGLShader::UploadUniformFloat(const std::string& name, const float value) const
	{
		const GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniform1f(location, value);
	}

	void OpenGLShader::UploadUniformFloat2(const std::string& name, const glm::vec2& vector) const
	{
		const GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniform2f(location, vector.x, vector.y);
	}

	void OpenGLShader::UploadUniformFloat3(const std::string& name, const glm::vec3& vector) const
	{
		const GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniform3f(location, vector.x, vector.y, vector.z);
	}

	void OpenGLShader::UploadUniformFloat4(const std::string& name, const glm::vec4& vector) const
	{
		const GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniform4f(location, vector.x, vector.y, vector.z, vector.w);
	}

	void OpenGLShader::UploadUniformMat3(const std::string& name, const glm::mat3& matrix) const
	{
		const GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniformMatrix3fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
	}

	void OpenGLShader::UploadUniformMat4(const std::string& name, const glm::mat4& matrix) const
	{
		const GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
	}

	std::string OpenGLShader::ReadFile(const std::string& filepath)
	{
		std::ifstream in(filepath, std::ios::in | std::ios::binary);

		if (!in)
		{
			KBR_CORE_ERROR("Could not open file '{0}'", filepath);
			KBR_CORE_ASSERT(false, "File not found!")
				return "";
		}

		/// Check the size of the file and resize the string
		std::string result;
		in.seekg(0, std::ios::end);
		result.resize(in.tellg());

		/// Read the shaders into the string
		in.seekg(0, std::ios::beg);
		in.read(result.data(), static_cast<std::streamsize>(result.size()));

		in.close();

		return result;
	}

	std::unordered_map<GLenum, std::string> OpenGLShader::Preprocess(const std::string& shaderSource)
	{
		std::unordered_map<GLenum, std::string> shaderSources;

		const char* typeToken = "#type";
		const size_t typeTokenLength = strlen(typeToken);
		size_t pos = shaderSource.find(typeToken, 0);

		while (pos != std::string::npos)
		{
			const size_t eol = shaderSource.find_first_of("\r\n", pos);
			KBR_CORE_ASSERT(eol != std::string::npos, "Syntax error!");
			const size_t begin = pos + typeTokenLength + 1;
			std::string type = shaderSource.substr(begin, eol - begin);
			KBR_CORE_ASSERT(type == "vertex" || type == "fragment" || type == "pixel" || type == "geometry", "Invalid shader type specified!");

			const size_t nextLinePos = shaderSource.find_first_not_of("\r\n", eol);
			pos = shaderSource.find(typeToken, nextLinePos);

			shaderSources[Utils::ShaderTypeFromString(type)]
				= shaderSource.substr(nextLinePos, pos - (nextLinePos == std::string::npos ? shaderSource.size() - 1 : nextLinePos));
		}

		return shaderSources;
	}

	void OpenGLShader::Compile(const std::unordered_map<GLenum, std::string>& shaderSources)
	{
		// Create a shader program
		const GLuint program = glCreateProgram();
		KBR_CORE_ASSERT(shaderSources.size() <= 3, "Only 3 shaders in a file is supported!");

		std::vector<GLenum> glShaderIDs;
		glShaderIDs.resize(shaderSources.size());

		int glShaderIDIndex = 0;
		for (const auto& [shaderType, source] : shaderSources)
		{
			const GLuint shader = glCreateShader(shaderType);

			const GLchar* shaderSource = source.c_str();
			glShaderSource(shader, 1, &shaderSource, nullptr);

			glCompileShader(shader);

			GLint isCompiled = 0;
			glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);
			if (isCompiled == GL_FALSE)
			{
				GLint maxLength = 0;
				glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

				std::vector<GLchar> infoLog(maxLength);
				glGetShaderInfoLog(shader, maxLength, &maxLength, infoLog.data());

				glDeleteShader(shader);

				KBR_CORE_ERROR("{0}", infoLog.data());
				KBR_CORE_ASSERT(false, "Shader compilation failure!")
					return;
			}

			// Attach the shader to our program
			glAttachShader(program, shader);

			glShaderIDs[glShaderIDIndex++] = shader;
		}

		// Link our program
		glLinkProgram(program);

		GLint isLinked = 0;
		glGetProgramiv(program, GL_LINK_STATUS, &isLinked);
		if (isLinked == GL_FALSE)
		{
			GLint maxLength = 0;
			glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);

			std::vector<GLchar> infoLog(maxLength);
			glGetProgramInfoLog(program, maxLength, &maxLength, infoLog.data());

			glDeleteProgram(program);

			for (const auto id : glShaderIDs)
				glDeleteShader(id);

			KBR_CORE_ERROR("{0}", infoLog.data());
			KBR_CORE_ASSERT(false, "Shader link failure!")
				return;
		}

		// Detach shaders after a successful link
		for (const auto id : glShaderIDs)
		{
			glDetachShader(program, id);
		}

		// Assign the programId to the class member only when compilation succeeded
		m_RendererID = program;
	}

	void OpenGLShader::CompileOrGetVulkanBinaries(const std::unordered_map<GLenum, std::string>& shaderSources) 
	{
		KBR_CORE_INFO("\nCompiling shader: {}", m_FilePath);

		shaderc::Compiler compiler;
		shaderc::CompileOptions options;
		options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_4);
		/// Turning on optimization will cause the shaders to have linking issues
		if (constexpr bool optimize = false)
			options.SetOptimizationLevel(shaderc_optimization_level_performance);

		std::filesystem::path cacheDirectory = Utils::GetCacheDirectory();

		auto& shaderData = m_VulkanSPIRV;
		shaderData.clear();
		for (auto&& [stage, source] : shaderSources)
		{
			std::filesystem::path shaderFilePath = m_FilePath;
			std::filesystem::path cachedPath = cacheDirectory / (shaderFilePath.filename().string() + Utils::GLShaderStageCachedVulkanFileExtension(stage));

			std::ifstream in(cachedPath, std::ios::in | std::ios::binary);
			if (in.is_open())
			{
				in.seekg(0, std::ios::end);
				auto size = in.tellg();
				in.seekg(0, std::ios::beg);

				auto& data = shaderData[stage];
				data.resize(size / sizeof(uint32_t));
				in.read(reinterpret_cast<char*>(data.data()), size);
			}
			else
			{
				shaderc::SpvCompilationResult module = compiler.CompileGlslToSpv(source, Utils::GLShaderStageToShaderC(stage), m_FilePath.c_str(), options);
				if (module.GetCompilationStatus() != shaderc_compilation_status_success)
				{
					KBR_CORE_ERROR(module.GetErrorMessage());
					const std::string message = std::format("Shader compilation failed to vulkan binary! Stage: {}, File: {}", Utils::GLShaderStageToString(stage), m_FilePath);
					KBR_CORE_ASSERT(false, message);
					return;
				}

				shaderData[stage] = std::vector<uint32_t>(module.cbegin(), module.cend());

				std::ofstream out(cachedPath, std::ios::out | std::ios::binary);
				if (out.is_open())
				{
					auto& data = shaderData[stage];
					out.write(reinterpret_cast<char*>(data.data()), data.size() * sizeof(uint32_t));
					out.flush();
					out.close();
				}
			}
		}

		for (auto&& [stage, data] : shaderData)
			Reflect(stage, data);
	}

	void OpenGLShader::CompileOrGetOpenGLBinaries() 
	{
		auto& shaderData = m_OpenGLSPIRV;

		shaderc::Compiler compiler;
		shaderc::CompileOptions options;
		options.SetTargetEnvironment(shaderc_target_env_opengl, shaderc_env_version_opengl_4_5);
		options.SetAutoBindUniforms(true);
		//options.SetAutoMapLocations(true);
		//options.SetSourceLanguage(shaderc_source_language_glsl);
		//options.SetVulkanRulesRelaxed(true);
		//options.SetForcedVersionProfile(450, shaderc_profile_core);
		/// Turning on optimization will cause the shaders to have linking issues
		if (constexpr bool optimize = false)
			options.SetOptimizationLevel(shaderc_optimization_level_performance);

		std::filesystem::path cacheDirectory = Utils::GetCacheDirectory();

		shaderData.clear();
		m_OpenGLSourceCode.clear();
		for (auto&& [stage, spirv] : m_VulkanSPIRV)
		{
			std::filesystem::path shaderFilePath = m_FilePath;
			std::filesystem::path cachedPath = cacheDirectory / (shaderFilePath.filename().string() + Utils::GLShaderStageCachedOpenGLFileExtension(stage));

			std::ifstream in(cachedPath, std::ios::in | std::ios::binary);
			if (in.is_open())
			{
				in.seekg(0, std::ios::end);
				auto size = in.tellg();
				in.seekg(0, std::ios::beg);

				auto& data = shaderData[stage];
				data.resize(size / sizeof(uint32_t));
				in.read(reinterpret_cast<char*>(data.data()), size);
			}
			else
			{
				spirv_cross::CompilerGLSL glslCompiler(spirv);
				/*spirv_cross::CompilerGLSL::Options glslOptions = glslCompiler.get_common_options();
				glslOptions.emit_push_constant_as_uniform_buffer = false;
				glslOptions.vulkan_semantics = false;
				glslCompiler.set_common_options(glslOptions);*/

				m_OpenGLSourceCode[stage] = glslCompiler.compile();
				auto& source = m_OpenGLSourceCode[stage];

				shaderc::SpvCompilationResult module = compiler.CompileGlslToSpv(source, Utils::GLShaderStageToShaderC(stage), m_FilePath.c_str(), options);
				if (module.GetCompilationStatus() != shaderc_compilation_status_success)
				{
					KBR_CORE_ERROR(module.GetErrorMessage());
					const std::string message = std::format("Shader compilation failed to opengl binary! Stage: {}, File: {}", Utils::GLShaderStageToString(stage), m_FilePath);
					KBR_CORE_ASSERT(false, message);
					return;
				}

				shaderData[stage] = std::vector<uint32_t>(module.cbegin(), module.cend());

				std::ofstream out(cachedPath, std::ios::out | std::ios::binary);
				if (out.is_open())
				{
					auto& data = shaderData[stage];
					out.write(reinterpret_cast<char*>(data.data()), data.size() * sizeof(uint32_t));
					out.flush();
					out.close();
				}
			}
		}
	}

	void OpenGLShader::CreateProgram() 
	{
		const GLuint program = glCreateProgram();

		std::vector<GLuint> shaderIDs;
		for (auto&& [stage, spirv] : m_OpenGLSPIRV)
		{
			GLuint shaderID = shaderIDs.emplace_back(glCreateShader(stage));
			glShaderBinary(1, &shaderID, GL_SHADER_BINARY_FORMAT_SPIR_V, spirv.data(), spirv.size() * sizeof(uint32_t));
			glSpecializeShader(shaderID, "main", 0, nullptr, nullptr);
			glAttachShader(program, shaderID);
		}

		glLinkProgram(program);

		GLint isLinked;
		glGetProgramiv(program, GL_LINK_STATUS, &isLinked);
		if (isLinked == GL_FALSE)
		{
			GLint maxLength;
			glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);

			std::vector<GLchar> infoLog(maxLength);
			glGetProgramInfoLog(program, maxLength, &maxLength, infoLog.data());
			KBR_CORE_ERROR("Shader linking failed ({0}):\n{1}", m_FilePath, infoLog.data());

			glDeleteProgram(program);

			for (const auto id : shaderIDs)
				glDeleteShader(id);
		}

		for (const auto id : shaderIDs)
		{
			glDetachShader(program, id);
			glDeleteShader(id);
		}

		m_RendererID = program;
	}

	void OpenGLShader::Reflect(const GLenum stage, const std::vector<uint32_t>& shaderData) 
	{
		const spirv_cross::Compiler compiler(shaderData);
		spirv_cross::ShaderResources resources = compiler.get_shader_resources();

		const char* stageName = nullptr;
		switch (stage)
		{
		case GL_VERTEX_SHADER:   stageName = "Vertex"; break;
		case GL_FRAGMENT_SHADER: stageName = "Fragment"; break;
		case GL_GEOMETRY_SHADER: stageName = "Geometry"; break;
		}

		KBR_CORE_TRACE("  Stage: {0}", stageName);
		KBR_CORE_TRACE("    Uniform Buffers: {0}", resources.uniform_buffers.size());
		for (const auto& resource : resources.uniform_buffers)
		{
			const auto& bufferType = compiler.get_type(resource.base_type_id);
			uint32_t set = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
			uint32_t binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
			size_t bufferSize = compiler.get_declared_struct_size(bufferType);

			KBR_CORE_TRACE("      Name: {0}, Set: {1}, Binding: {2}, Size: {3}", resource.name, set, binding, bufferSize);
		}

		KBR_CORE_TRACE("    Sampled Images (Textures/Samplers): {0}", resources.sampled_images.size());
		for (const auto& resource : resources.sampled_images)
		{
			uint32_t set = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
			uint32_t binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
			uint32_t descriptorCount = 1;

			// Check if it's an array of textures (e.g., `sampler2D textures[4]`)
			const spirv_cross::SPIRType& type = compiler.get_type(resource.base_type_id);
			if (!type.array.empty())
			{
				descriptorCount = type.array[0]; // Assuming 1D array for simplicity
				if (descriptorCount == 0) // Unsized array (e.g., `sampler2D textures[]`)
					//descriptorCount = VulkanContext::Get().GetCapabilities().maxSamplerAllocationCount; // Or some max you define
					descriptorCount = 1; // Default to 1 if unsized
			}

			KBR_CORE_TRACE("      Name: {0}, Set: {1}, Binding: {2}, Count: {3}", resource.name, set, binding, descriptorCount);
		}

		KBR_CORE_TRACE("    Storage Buffers: {0}", resources.storage_buffers.size());
		for (const auto& resource : resources.storage_buffers)
		{
			const auto& bufferType = compiler.get_type(resource.base_type_id);
			uint32_t set = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
			uint32_t binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
			size_t bufferSize = compiler.get_declared_struct_size(bufferType);

			KBR_CORE_TRACE("      Name: {0}, Set: {1}, Binding: {2}, Approx Size: {3}", resource.name, set, binding, bufferSize);
		}

		KBR_CORE_TRACE("    Storage Images: {0}", resources.storage_images.size());
		for (const auto& resource : resources.storage_images)
		{
			uint32_t set = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
			uint32_t binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
			uint32_t descriptorCount = 1;
			const spirv_cross::SPIRType& type = compiler.get_type(resource.base_type_id);
			if (!type.array.empty())
			{
				descriptorCount = type.array[0];
				if (descriptorCount == 0)
					//descriptorCount = VulkanContext::Get().GetCapabilities().maxStorageImageAllocationCount; // Or some max
					descriptorCount = 1; // Default to 1 if unsized
			}

			KBR_CORE_TRACE("      Name: {0}, Set: {1}, Binding: {2}", resource.name, set, binding);
		}

		KBR_CORE_TRACE("    Push Constant Buffers: {0}", resources.push_constant_buffers.size());
		for (const auto& resource : resources.push_constant_buffers)
		{
			const auto& bufferType = compiler.get_type(resource.base_type_id);
			uint32_t offset = 0; // Typically
			// SPIRV-Cross might need a bit more work to get exact offset for members if it's a struct.
		   // For a single push constant block, its range covers the whole block.
			size_t size = compiler.get_declared_struct_size(bufferType);

			// Get shader stage for this push constant more accurately
			// auto ranges = compiler.get_active_buffer_ranges(resource.id);
			// For now, we assume the 'stage' passed to Reflect applies.

			KBR_CORE_TRACE("      Name: {0}, Offset: {1}, Size: {2}", resource.name, offset, size);
		}
	}
}
