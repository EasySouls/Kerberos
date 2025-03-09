#pragma once

#include "Kerberos/Renderer/Shader.h"
#include <unordered_map>
#include <glm/glm.hpp>


namespace Kerberos
{
	class OpenGLShader final : public Shader
	{
	public:
		explicit OpenGLShader(const std::string& filepath);
		OpenGLShader(std::string name, const std::string& vertexSrc, const std::string& fragmentSrc);
		~OpenGLShader() override;

		void Bind() const override;
		void Unbind() const override;

		const std::string& GetName() const override { return m_Name; }

		void SetInt(const std::string& name, int value) override;
		void SetIntArray(const std::string& name, int* values, uint32_t count) override;
		void SetFloat(const std::string& name, float value) override;
		void SetFloat3(const std::string& name, const glm::vec3& value) override;
		void SetFloat4(const std::string& name, const glm::vec4& value) override;
		void SetMat4(const std::string& name, const glm::mat4& value) override;

	private:
		void UploadUniformInt(const std::string& name, int value) const;
		void UploadUniformIntArray(const std::string& name, const int* values, uint32_t count) const;

		void UploadUniformFloat(const std::string& name, float value) const;
		void UploadUniformFloat2(const std::string& name, const glm::vec2& vector) const;
		void UploadUniformFloat3(const std::string& name, const glm::vec3& vector) const;
		void UploadUniformFloat4(const std::string& name, const glm::vec4& vector) const;

		void UploadUniformMat3(const std::string& name, const glm::mat3& matrix) const;
		void UploadUniformMat4(const std::string& name, const glm::mat4& matrix) const;
		std::string ReadFile(const std::string& filepath);
		std::unordered_map<unsigned int, std::string> Preprocess(const std::string& shaderSource);
		void Compile(const std::unordered_map<unsigned int, std::string>& shaderSources);
	
	private:
		uint32_t m_RendererID;
		std::string m_Name;
	};
}
