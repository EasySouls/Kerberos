#pragma once

#include <glm/glm.hpp>

#include "Buffer.h"

namespace Kerberos
{
	struct Vertex
	{
		glm::vec3 Position;
		glm::vec3 Normal;
		glm::vec2 TexCoord;

		static BufferLayout GetLayout()
		{
			return BufferLayout
			{
				{ ShaderDataType::Float3, "a_Position" },
				{ ShaderDataType::Float3, "a_Normal"   },
				{ ShaderDataType::Float2, "a_TexCoord" },
			};
		}
	};

	struct TextVertex
	{
		glm::vec3 Position;
		glm::vec4 Color;
		glm::vec2 TexCoord;

		int EntityID = -1;

		static BufferLayout GetLayout()
		{
			return BufferLayout
			{
				{ ShaderDataType::Float3, "a_Position" },
				{ ShaderDataType::Float4, "a_Color"    },
				{ ShaderDataType::Float2, "a_TexCoord" },
				{ ShaderDataType::Int,	"a_EntityID" }
			};
		}
	};
}
