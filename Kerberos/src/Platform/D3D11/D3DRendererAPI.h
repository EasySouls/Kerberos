#pragma once

#include "Kerberos/Renderer/RendererAPI.h"

namespace Kerberos
{
	class D3DRendererAPI : public RendererAPI
	{
	public:
		void Init() override;

		void SetClearColor(const glm::vec4& color) override;
		void Clear() override;

		void DrawIndexed(const Ref<VertexArray>& vertexArray) override;
	};
}
