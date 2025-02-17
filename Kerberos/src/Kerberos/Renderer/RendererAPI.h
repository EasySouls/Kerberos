#pragma once

#include "Kerberos/Core.h"
#include "VertexArray.h"

#include <glm/glm.hpp>

namespace Kerberos
{
	class RendererAPI
	{
	public:
		enum class API : uint8_t
		{
			None = 0, 
			OpenGL = 1, 
			Vulkan = 2
		};

		virtual void SetClearColor(const glm::vec4& color) = 0;
		virtual void Clear() = 0;

		virtual void DrawIndexed(const Ref<VertexArray>& vertexArray) = 0;

		static API GetAPI() { return s_API; }
	private:
		static API s_API;
	};
}