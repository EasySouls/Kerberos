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
			Vulkan = 2,
		};

		virtual ~RendererAPI() = default;

		virtual void Init() = 0;

		virtual void SetViewport(const uint32_t x, const uint32_t y, const uint32_t width, const uint32_t height) = 0;

		virtual void SetClearColor(const glm::vec4& color) = 0;
		virtual void Clear() = 0;

		virtual void DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t indexCount = 0) = 0;

		static API GetAPI() { return s_API; }
	private:
		static API s_API;
	};
}