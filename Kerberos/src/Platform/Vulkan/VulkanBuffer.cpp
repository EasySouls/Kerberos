#include "kbrpch.h"
#include "VulkanBuffer.h"

namespace Kerberos
{
	VulkanVertexBuffer::VulkanVertexBuffer(const float* vertices, uint32_t size)
	{
		throw std::logic_error("Not implemented");
	}

	VulkanVertexBuffer::VulkanVertexBuffer(uint32_t size)
	{
		throw std::logic_error("Not implemented");
	}

	void VulkanVertexBuffer::Bind() const
	{
		throw std::logic_error("Not implemented");
	}

	void VulkanVertexBuffer::Unbind() const
	{
		throw std::logic_error("Not implemented");
	}

	void VulkanVertexBuffer::SetData(const void* data, uint32_t size)
	{
		throw std::logic_error("Not implemented");
	}

	void VulkanVertexBuffer::SetLayout(const BufferLayout& layout)
	{
		throw std::logic_error("Not implemented");
	}

	const BufferLayout& VulkanVertexBuffer::GetLayout() const
	{
		throw std::logic_error("Not implemented");
	}

	VulkanIndexBuffer::VulkanIndexBuffer(const uint32_t* indices, uint32_t count)
	{
		throw std::logic_error("Not implemented");
	}

	void VulkanIndexBuffer::Bind() const
	{
		throw std::logic_error("Not implemented");
	}

	void VulkanIndexBuffer::Unbind() const
	{
		throw std::logic_error("Not implemented");
	}

	uint32_t VulkanIndexBuffer::GetCount() const
	{
		throw std::logic_error("Not implemented");
	}
}
