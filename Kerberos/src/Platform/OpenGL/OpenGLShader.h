#pragma once

#include "Kerberos/Renderer/Shader.h"
#include <unordered_map>
#include <glm/glm.hpp>


namespace Kerberos
{
	class OpenGLShader : public Shader
	{
	public:
		OpenGLShader(const std::string& filepath);
		OpenGLShader(const std::string& vertexSrc, const std::string& fragmentSrc);
		~OpenGLShader() override;

		void Bind() const override;
		void Unbind() const override;

		void UploadUniformInt(const std::string& name, int value) const;

		void UploadUniformFloat(const std::string& name, float value) const;
		void UploadUniformFloat2(const std::string& name, const glm::vec2& vector) const;
		void UploadUniformFloat3(const std::string& name, const glm::vec3& vector) const;
		void UploadUniformFloat4(const std::string& name, const glm::vec4& vector) const;

		void UploadUniformMat3(const std::string& name, const glm::mat3& matrix) const;
		void UploadUniformMat4(const std::string& name, const glm::mat4& matrix) const;
	private:
		std::string ReadFile(const std::string& filepath);
		std::unordered_map<unsigned int, std::string> Preprocess(const std::string& shaderSource);
		void Compile(const std::unordered_map<unsigned int, std::string>& shaderSources);
	private:
		uint32_t m_RendererID;
	};
}
