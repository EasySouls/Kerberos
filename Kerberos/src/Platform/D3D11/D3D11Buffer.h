#pragma once

#include "Kerberos/Renderer/Buffer.h"

namespace Kerberos
{
	class D3D11VertexBuffer final : public VertexBuffer
	{
	public:
		D3D11VertexBuffer(const float* vertices, uint32_t size);
		explicit D3D11VertexBuffer(uint32_t size);
		~D3D11VertexBuffer() override;

		void SetData(const void* data, uint32_t size) override;

		void Bind() const override;
		void Unbind() const override;

		void SetLayout(const BufferLayout& layout) override { m_Layout = layout; }
		const BufferLayout& GetLayout() const override { return m_Layout; }

	private:
		uint32_t m_RendererID;
		BufferLayout m_Layout;
	};

	class D3D11IndexBuffer final : public IndexBuffer
	{
	public:
		D3D11IndexBuffer(const uint32_t* indices, uint32_t count);
		~D3D11IndexBuffer() override;

		void Bind() const override;
		void Unbind() const override;

		uint32_t GetCount() const override { return m_Count; }

	private:
		uint32_t m_RendererID;
		uint32_t m_Count;
	};
	
}

