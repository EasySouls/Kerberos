#include "kbrpch.h"
#include "VulkanBuffer.h"

namespace Kerberos
{
	VulkanVertexBuffer::VulkanVertexBuffer(const float* vertices, uint32_t size)
	{
	}

	VulkanVertexBuffer::VulkanVertexBuffer(uint32_t size)
	{
	}

	void VulkanVertexBuffer::Bind() const
	{
	}

	void VulkanVertexBuffer::Unbind() const
	{
	}

	void VulkanVertexBuffer::SetData(const void* data, uint32_t size)
	{
	}

	void VulkanVertexBuffer::SetLayout(const BufferLayout& layout)
	{
	}

	const BufferLayout& VulkanVertexBuffer::GetLayout() const
	{
		return BufferLayout(); /// TODO: Placeholder return value, replace with actual layout
	}

	VulkanIndexBuffer::VulkanIndexBuffer(const uint32_t* indices, uint32_t count)
	{
	}

	void VulkanIndexBuffer::Bind() const
	{
	}

	void VulkanIndexBuffer::Unbind() const
	{
	}

	uint32_t VulkanIndexBuffer::GetCount() const
	{
		return 0; /// TODO: Placeholder return value, replace with actual count
	}
}
