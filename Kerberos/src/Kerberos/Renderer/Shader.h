#pragma once

#include <string>
#include <glm/glm.hpp>

namespace Kerberos
{
	class Shader
	{
	public:
		Shader(const std::string& vertexSrc, const std::string& fragmentSrc);
		virtual ~Shader();

		virtual void Bind() const;
		virtual void Unbind() const;

		void UploadUniformMat4(const std::string& name, const glm::mat4& matrix) const;
	private:
		uint32_t m_RendererID;
	};
}