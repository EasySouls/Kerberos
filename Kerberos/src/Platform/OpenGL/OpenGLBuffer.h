#pragma once

#include "Kerberos/Renderer/Buffer.h"

namespace Kerberos
{
	class OpenGLVertexBuffer final : public VertexBuffer
	{
	public:
		OpenGLVertexBuffer(const float* vertices, uint32_t size);
		explicit OpenGLVertexBuffer(uint32_t size);
		~OpenGLVertexBuffer() override;

		void SetData(const void* data, uint32_t size) override;

		void Bind() const override;
		void Unbind() const override;

		void SetLayout(const BufferLayout& layout) override { m_Layout = layout; }
		const BufferLayout& GetLayout() const override { return m_Layout; }
		uint32_t GetCount() const override { return m_Count; }

	private:
		uint32_t m_RendererID;
		BufferLayout m_Layout;
		uint32_t m_Count;
	};

	class OpenGLIndexBuffer final : public IndexBuffer
	{
	public:
		OpenGLIndexBuffer(const uint32_t* indices, uint32_t count);
		~OpenGLIndexBuffer() override;

		void Bind() const override;
		void Unbind() const override;

		uint32_t GetCount() const override { return m_Count; }

	private:
		uint32_t m_RendererID;
		uint32_t m_Count;
	};
}

