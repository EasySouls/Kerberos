#include "kbrpch.h"
#include "OpenGLShader.h"

#include "Kerberos/Core.h"
#include "Kerberos/Core/Timer.h"

#include <fstream>
#include <filesystem>

#include <glm/gtc/type_ptr.hpp>
#include <glad/glad.h>

#include <shaderc/shaderc.hpp>
#include <spirv_cross/spirv_glsl.hpp>
#include <spirv_cross/spirv_cross.hpp>



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

			KBR_CORE_ASSERT(false, "Unknown shader type!")
			return 0;
		}

		static shaderc_shader_kind GLShaderStageToShaderC(const GLenum stage)
		{
			switch (stage)
			{
				case GL_VERTEX_SHADER:   return shaderc_glsl_vertex_shader;
				case GL_FRAGMENT_SHADER: return shaderc_glsl_fragment_shader;
				default: break;
			}
			KBR_CORE_ASSERT(false, "Unknown shader stage!")
			return static_cast<shaderc_shader_kind>(0);
		}

		static const char* GLShaderStageToString(const GLenum stage)
		{
			switch (stage)
			{
				case GL_VERTEX_SHADER:   return "GL_VERTEX_SHADER";
				case GL_FRAGMENT_SHADER: return "GL_FRAGMENT_SHADER";
				default: break;
			}

			KBR_CORE_ASSERT(false, "Only fragment and vertex shaders are supported for now!")
			return nullptr;
		}

		static const char* GetCacheDirectory()
		{
			return "assets/shadercache/opengl";
		}

		static void CreateCacheDirectoryIfNeeded()
		{
			const std::string cacheDirectory = GetCacheDirectory();
			if (!std::filesystem::exists(cacheDirectory))
				std::filesystem::create_directories(cacheDirectory);
		}

		static const char* GLShaderStageCachedOpenGLFileExtension(const GLenum stage)
		{
			switch (stage)
			{
				case GL_VERTEX_SHADER:   return ".cached_opengl.vert";
				case GL_FRAGMENT_SHADER: return ".cached_opngl.frag";
				default: break;
			}

			KBR_CORE_ASSERT(false, "Only fragment and vertex shaders are supported for now!")
			return "";
		}

		static const char* GLShaderStageCachedVulkanFileExtension(const GLenum stage)
		{
			switch (stage)
			{
				case GL_VERTEX_SHADER:   return ".cached_vulkan.vert";
				case GL_FRAGMENT_SHADER: return ".cached_vulkan.frag";
				default: break;
			}
			KBR_CORE_ASSERT(false, "Only fragment and vertex shaders are supported for now!")
			return "";
		}
	}

	OpenGLShader::OpenGLShader(const std::string& filepath)
		: m_FilePath(filepath)
	{
		Utils::CreateCacheDirectoryIfNeeded();
		const std::string source = ReadFile(filepath);
		const auto shaderSources = Preprocess(source);

		{
			const Timer timer;
			CompileOrGetVulkanBinaries(shaderSources);
			CompileOrGetOpenGLBinaries();
			CreateProgram();
			KBR_CORE_WARN("Shader compilation took {0}ms", timer.ElapsedMillis());
		}

		const std::filesystem::path path = filepath;
		m_Name = path.stem().string();
	}

	OpenGLShader::OpenGLShader(std::string name, const std::string& vertexSrc, const std::string& fragmentSrc)
		: m_Name(std::move(name))
	{
		std::unordered_map<GLenum, std::string> sources;
		sources[GL_VERTEX_SHADER] = vertexSrc;
		sources[GL_FRAGMENT_SHADER] = fragmentSrc;
		
		CompileOrGetVulkanBinaries(sources);
		CompileOrGetOpenGLBinaries();
		CreateProgram();
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

	void OpenGLShader::SetIntArray(const std::string& name, int* values, uint32_t count)
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
			KBR_CORE_ERROR("Could not open file '{0}'", filepath.c_str());
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
			KBR_CORE_ASSERT(type == "vertex" || type == "fragment" || type == "pixel", "Invalid shader type specified!");

			const size_t nextLinePos = shaderSource.find_first_not_of("\r\n", eol);
			pos = shaderSource.find(typeToken, nextLinePos);

			shaderSources[Utils::ShaderTypeFromString(type)] 
				= shaderSource.substr(nextLinePos, pos - (nextLinePos == std::string::npos ? shaderSource.size() - 1 : nextLinePos));
		}

		return shaderSources;
	}

	void OpenGLShader::CompileOrGetVulkanBinaries(const std::unordered_map<GLenum, std::string>& shaderSources)
	{
		shaderc::Compiler compiler;
		shaderc::CompileOptions options;
		options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_2);

		if (constexpr bool optimize = true)
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
				const size_t size = in.tellg();
				in.seekg(0, std::ios::beg);

				auto& data = shaderData[stage];
				data.resize(size / sizeof(uint32_t));
				in.read(reinterpret_cast<char*>(data.data()), static_cast<std::streamsize>(size));
			}
			else
			{
				shaderc::SpvCompilationResult module = compiler.CompileGlslToSpv(source, Utils::GLShaderStageToShaderC(stage), m_FilePath.c_str(), options);
				if (module.GetCompilationStatus() != shaderc_compilation_status_success)
				{
					KBR_CORE_ERROR(module.GetErrorMessage());
					KBR_CORE_ASSERT(false, "Shader compilation failure!");
				}

				shaderData[stage] = { module.cbegin(), module.cend() };

				std::ofstream out(cachedPath, std::ios::out | std::ios::binary);
				if (out.is_open())
				{
					out.write(reinterpret_cast<const char*>(shaderData[stage].data()), static_cast<std::streamsize>(shaderData[stage].size() * sizeof(uint32_t)));
				}
				else
				{
					auto& data = shaderData[stage];
					out.write(reinterpret_cast<char*>(data.data()), static_cast<std::streamsize>(data.size() * sizeof(uint32_t)));
					out.flush();
					out.close();
				}
			}
		}

		for (auto&& [stage, data] : shaderData)
		{
			/*const GLuint shader = glCreateShader(stage);
			const uint32_t* source = data.data();
			const int size = static_cast<int>(data.size() * sizeof(uint32_t));
			glShaderBinary(1, &shader, GL_SHADER_BINARY_FORMAT_SPIR_V, source, size);
			glSpecializeShader(shader, "main", 0, nullptr, nullptr);
			glAttachShader(program, shader);
			glDeleteShader(shader);*/
			Reflect(stage, data);
		}
	}

	void OpenGLShader::CompileOrGetOpenGLBinaries()
	{
		auto& shaderData = m_OpenGLSPIRV;

		shaderc::CompileOptions options;
		options.SetTargetEnvironment(shaderc_target_env_opengl, shaderc_env_version_opengl_4_5);
		if (constexpr bool optimize = true)
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
				shaderc::Compiler compiler;
				spirv_cross::CompilerGLSL glslCompiler(spirv);
				m_OpenGLSourceCode[stage] = glslCompiler.compile();
				auto& source = m_OpenGLSourceCode[stage];

				shaderc::SpvCompilationResult module = compiler.CompileGlslToSpv(source, Utils::GLShaderStageToShaderC(stage), m_FilePath.c_str());
				if (module.GetCompilationStatus() != shaderc_compilation_status_success)
				{
					KBR_CORE_ERROR(module.GetErrorMessage());
					KBR_CORE_ASSERT(false, "Failed to compile OpenGL shader binaries")
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
			const GLuint shaderID = shaderIDs.emplace_back(glCreateShader(stage));

			glShaderBinary(1, &shaderID, GL_SHADER_BINARY_FORMAT_SPIR_V, spirv.data(), static_cast<int>(spirv.size()) * sizeof(uint32_t));
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
			glDeleteProgram(program);

			for (const auto id : shaderIDs)
				glDeleteShader(id);

			KBR_CORE_ERROR("Shader linking failed ({0}):\n\t{1}", m_FilePath, infoLog.data());
			KBR_CORE_ASSERT(false, "Shader link failure!");
			return;
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
		const spirv_cross::ShaderResources resources = compiler.get_shader_resources();

		KBR_CORE_TRACE("OpenGLShader::Reflect - {0}", Utils::GLShaderStageToString(stage));
		KBR_CORE_TRACE("OpenGLShader::Reflect - {0} uniform buffers", resources.uniform_buffers.size());
		KBR_CORE_TRACE("OpenGLShader::Reflect - {0} storage buffers", resources.storage_buffers.size());
		KBR_CORE_TRACE("OpenGLShader::Reflect - {0} push constant buffers", resources.push_constant_buffers.size());
		KBR_CORE_TRACE("OpenGLShader::Reflect - {0} separate images", resources.separate_images.size());
		KBR_CORE_TRACE("OpenGLShader::Reflect - {0} separate samplers", resources.separate_samplers.size());
		KBR_CORE_TRACE("OpenGLShader::Reflect - {0} sampled images", resources.sampled_images.size());
		KBR_CORE_TRACE("OpenGLShader::Reflect - {0} storage images", resources.storage_images.size());

		KBR_CORE_TRACE("Uniform buffers:");
		for (const auto& resource : resources.uniform_buffers)
		{
			const spirv_cross::SPIRType& bufferType = compiler.get_type(resource.base_type_id);
			//KBR_CORE_TRACE("  Type: {0}", bufferType);
			const auto& bufferName = resource.name;
			//KBR_CORE_TRACE("  Name: {0}", bufferName);
			const auto& bufferSize = compiler.get_declared_struct_size(bufferType);
			KBR_CORE_TRACE("  Size: {0}", bufferSize);
			for (const auto& member : bufferType.member_types)
			{
				const auto& memberType = compiler.get_type(member);
				//KBR_CORE_TRACE("    MemberType: {0}", memberType.self);
			}
		}
	}
}
