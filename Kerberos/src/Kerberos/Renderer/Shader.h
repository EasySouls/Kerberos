#pragma once

#include <string>

namespace Kerberos
{
	class Shader
	{
	public:
		Shader(const std::string& vertexSrc, const std::string& fragmentSrc);
		virtual ~Shader();

		virtual void Bind() const;
		virtual void Unbind() const;

	private:
		uint32_t m_RendererID;
	};
}