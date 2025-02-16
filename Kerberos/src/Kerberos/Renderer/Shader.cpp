#include "kbrpch.h"
#include "Shader.h"
#include "Kerberos/Core.h"

#include <glad/glad.h>

namespace Kerberos
{
	Shader::Shader(const std::string& vertexSrc, const std::string& fragmentSrc)
	{
		// Create an empty vertex shader handle
		const GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);

		// Send the vertex shader source code to GL
		const GLchar* source = vertexSrc.c_str();
		glShaderSource(vertexShader, 1, &source, nullptr);

		glCompileShader(vertexShader);

		GLint isCompiled = 0;
		glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &isCompiled);
		if (isCompiled == GL_FALSE)
		{
			GLint maxLength = 0;
			glGetShaderiv(vertexShader, GL_INFO_LOG_LENGTH, &maxLength);

			std::vector<GLchar> infoLog(maxLength);
			glGetShaderInfoLog(vertexShader, maxLength, &maxLength, infoLog.data());

			glDeleteShader(vertexShader);

			KBR_CORE_ERROR("{0}", infoLog.data());
			KBR_CORE_ASSERT(false, "Vertex shader compilation failure!")

			return;
		}

		// Create an empty fragment shader handle
		const GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

		// Send the fragment shader source code to GL
		source = fragmentSrc.c_str();
		glShaderSource(fragmentShader, 1, &source, nullptr);

		glCompileShader(fragmentShader);

		isCompiled = 0;
		glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &isCompiled);
		if (isCompiled == GL_FALSE)
		{
			GLint maxLength = 0;
			glGetShaderiv(fragmentShader, GL_INFO_LOG_LENGTH, &maxLength);

			std::vector<GLchar> infoLog(maxLength);
			glGetShaderInfoLog(fragmentShader, maxLength, &maxLength, infoLog.data());

			glDeleteShader(fragmentShader);
			glDeleteShader(vertexShader);

			KBR_CORE_ERROR("{0}", infoLog.data());
			KBR_CORE_ASSERT(false, "Fragment shader compilation failure!")

			return;
		}

		// Create a shader program
		m_RendererID = glCreateProgram();

		// Attach our shaders to our program
		glAttachShader(m_RendererID, vertexShader);
		glAttachShader(m_RendererID, fragmentShader);

		// Link our program
		glLinkProgram(m_RendererID);

		GLint isLinked = 0;
		glGetProgramiv(m_RendererID, GL_LINK_STATUS, &isLinked);
		if (isLinked == GL_FALSE)
		{
			GLint maxLength = 0;
			glGetProgramiv(m_RendererID, GL_INFO_LOG_LENGTH, &maxLength);

			std::vector<GLchar> infoLog(maxLength);
			glGetProgramInfoLog(m_RendererID, maxLength, &maxLength, infoLog.data());

			glDeleteProgram(m_RendererID);
			glDeleteShader(vertexShader);
			glDeleteShader(fragmentShader);

			KBR_CORE_ERROR("{0}", infoLog.data());
			KBR_CORE_ASSERT(false, "Shader link failure!")

			return;
		}

		// Detach shaders after a successful link
		glDetachShader(m_RendererID, vertexShader);
		glDetachShader(m_RendererID, fragmentShader);
	}

	Shader::~Shader()
	{
		glDeleteProgram(m_RendererID);
	}

	void Shader::Bind() const
	{
		glUseProgram(m_RendererID);
	}

	void Shader::Unbind() const
	{
		glUseProgram(0);
	}
}