#pragma once

#include "Kerberos/Renderer/Buffer.h"

#include <vulkan/vulkan.h>

namespace Kerberos
{
	class VulkanVertexBuffer final : public VertexBuffer
	{
	public:
		VulkanVertexBuffer(const float* vertices, uint32_t size);
		explicit VulkanVertexBuffer(uint32_t size);
		~VulkanVertexBuffer() override;

		void Bind() const override;
		void Unbind() const override;

		void SetData(const void* data, uint32_t size) override;

		void SetLayout(const BufferLayout& layout) override;
		const BufferLayout& GetLayout() const override;

		VkBuffer GetVkBuffer() const { return m_Buffer; }

	private:
		VkBuffer m_Buffer = VK_NULL_HANDLE;
		VkDeviceMemory m_BufferMemory = VK_NULL_HANDLE;
	};	

	class VulkanIndexBuffer final : public IndexBuffer
	{
	public:
		VulkanIndexBuffer(const uint32_t* indices, uint32_t count);
		~VulkanIndexBuffer() override;

		void Bind() const override;
		void Unbind() const override;

		uint32_t GetCount() const override;
		int GetType() const { return VK_INDEX_TYPE_UINT16; }

		VkBuffer GetVkBuffer() const { return m_Buffer; }

	private:
		VkBuffer m_Buffer = VK_NULL_HANDLE;
		VkDeviceMemory m_BufferMemory = VK_NULL_HANDLE;
		uint32_t m_Count = 0;
	};
}

