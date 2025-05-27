#pragma once

#include "Kerberos/Renderer/VertexArray.h"

// TODO: D3D11 doesn't use vertex arrays in the same way as OpenGL, so this class will need to be adapted accordingly.
// This is just written so Kerberos compiles until further refactoring is done.

namespace Kerberos
{
	class D3D11VertexArray final : public VertexArray
	{
	public:
		D3D11VertexArray();
		~D3D11VertexArray() override;

		void Bind() const override;
		void Unbind() const override;

		void AddVertexBuffer(const Ref<VertexBuffer>& vertexBuffer) override;
		void SetIndexBuffer(const Ref<IndexBuffer>& indexBuffer) override;

		const std::vector<Ref<VertexBuffer>>& GetVertexBuffers() const override { return m_VertexBuffers; }
		const Ref<IndexBuffer>& GetIndexBuffer() const override { return m_IndexBuffer; }
	private:
		uint32_t m_RendererID;
		std::vector<Ref<VertexBuffer>> m_VertexBuffers;
		Ref<IndexBuffer> m_IndexBuffer;
	};
	
}

