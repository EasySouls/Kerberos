#pragma once

#include "Kerberos/Renderer/Buffer.h"

namespace Kerberos
{
	class VulkanVertexBuffer final : public VertexBuffer
	{
	public:
		VulkanVertexBuffer(const float* vertices, uint32_t size);
		explicit VulkanVertexBuffer(uint32_t size);
		~VulkanVertexBuffer() override = default;

		void Bind() const override;
		void Unbind() const override;

		void SetData(const void* data, uint32_t size) override;

		void SetLayout(const BufferLayout& layout) override;
		const BufferLayout& GetLayout() const override;
	};	

	class VulkanIndexBuffer final : public IndexBuffer
	{
	public:
		VulkanIndexBuffer(const uint32_t* indices, uint32_t count);
		~VulkanIndexBuffer() override = default;

		void Bind() const override;
		void Unbind() const override;

		uint32_t GetCount() const override;
	};
}

