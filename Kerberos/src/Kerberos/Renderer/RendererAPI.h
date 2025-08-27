#pragma once

#include "Kerberos/Core.h"
#include "VertexArray.h"

#include <glm/glm.hpp>

namespace Kerberos
{
	enum class DepthFunc : uint8_t
	{
		Always = 0,
		Never = 1,
		Less = 2,
		LessEqual = 3,
		Greater = 4,
		GreaterEqual = 5,
		Equal = 6,
		NotEqual = 7
	};

	class RendererAPI
	{
	public:
		enum class API : uint8_t
		{
			OpenGL = 0, 
			D3D11 = 1,
			D3D12 = 2,
			Vulkan = 3,
		};

		virtual ~RendererAPI() = default;

		virtual void Init() = 0;

		virtual void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) = 0;

		virtual void SetClearColor(const glm::vec4& color) = 0;
		virtual void Clear() = 0;
		virtual void ClearDepth() = 0;

		virtual void SetDepthTest(bool enabled) = 0;
		virtual void SetDepthFunc(DepthFunc func) = 0;

		virtual void DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t indexCount = 0) = 0;
		virtual void DrawArray(const Ref<VertexArray>& vertexArray, uint32_t vertexCount) = 0;

		static API GetAPI() { return s_API; }
	private:
		static API s_API;
	};
}