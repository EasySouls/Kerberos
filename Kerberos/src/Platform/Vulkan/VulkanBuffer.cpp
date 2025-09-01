#include "kbrpch.h"
#include "VulkanBuffer.h"

#include "VulkanContext.h"

namespace Kerberos
{
	VulkanVertexBuffer::VulkanVertexBuffer(const float* vertices, const uint32_t size)
		: m_BufferSize(size)
	{
		KBR_PROFILE_FUNCTION();

		CreateBufferAndAllocateMemory(size);

		SetData(vertices, size);
	}

	VulkanVertexBuffer::VulkanVertexBuffer(const uint32_t size)
		: m_BufferSize(size)
	{
		KBR_PROFILE_FUNCTION();

		CreateBufferAndAllocateMemory(size);
	}

	VulkanVertexBuffer::~VulkanVertexBuffer()
	{
		KBR_PROFILE_FUNCTION();

		const VkDevice device = VulkanContext::Get().GetDevice();

		vkDeviceWaitIdle(device);

		vmaDestroyBuffer(VulkanContext::Get().GetAllocator().get(), m_Buffer, m_BufferAllocation);
		/*vkDestroyBuffer(device, m_Buffer, nullptr);
		vkFreeMemory(device, m_BufferMemory, nullptr);*/
	}

	void VulkanVertexBuffer::Bind() const
	{
		constexpr VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(VulkanContext::Get().GetCurrentCommandBuffer(), 0, 1, &m_Buffer, offsets);
	}

	void VulkanVertexBuffer::Unbind() const
	{
	}

	void VulkanVertexBuffer::SetData(const void* data, const uint32_t size)
	{
		KBR_PROFILE_FUNCTION();

		KBR_CORE_ASSERT(size <= m_BufferSize, "Data size is larger than buffer size!");
		KBR_CORE_ASSERT(m_AllocationInfo.pMappedData, "Buffer memory is not mapped!");

		/// TODO: This only works if we use the VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE with HOST_VISIBLE and HOST_COHERENT bits
		std::memcpy(m_AllocationInfo.pMappedData, data, size);

		/*void* data;
		vmaMapMemory(VulkanContext::Get().GetAllocator().get(), m_BufferAllocation, &data);
		memcpy(data, vertices, sizeof(float) * size);
		vmaUnmapMemory(VulkanContext::Get().GetAllocator().get(), m_BufferAllocation);*/
	}

	void VulkanVertexBuffer::SetLayout(const BufferLayout& layout)
	{
		m_Layout = layout;
	}

	const BufferLayout& VulkanVertexBuffer::GetLayout() const
	{
		return m_Layout;
	}

	void VulkanVertexBuffer::CreateBufferAndAllocateMemory(const uint32_t size) 
	{
		/// TODO: Add options for different usages and memory properties

		if (size == 0)
		{
			KBR_CORE_ERROR("Vertex buffer size is 0!");
			KBR_CORE_ASSERT(false, "Vertex buffer size is 0!");
			return;
		}

		const VkDevice device = VulkanContext::Get().GetDevice();

		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = size;
		bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		if (const VkResult result = vkCreateBuffer(device, &bufferInfo, nullptr, &m_Buffer); result != VK_SUCCESS)
		{
			KBR_CORE_ERROR("Failed to create vertex buffer: {}", VulkanHelpers::VkResultToString(result));
			KBR_CORE_ASSERT(false, "Failed to create vertex buffer: {}", VulkanHelpers::VkResultToString(result));
			return;
		}

		VmaAllocationCreateInfo allocationCi = VmaAllocationCreateInfo{};
		allocationCi.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;
		allocationCi.usage = VMA_MEMORY_USAGE_AUTO_PREFER_HOST;
		allocationCi.requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

		if (const VkResult result = vmaCreateBuffer(VulkanContext::Get().GetAllocator().get(), &bufferInfo, &allocationCi, &m_Buffer, &m_BufferAllocation, &m_AllocationInfo); result != VK_SUCCESS)
		{
			KBR_CORE_ERROR("Failed to allocate vertex buffer memory: {}", VulkanHelpers::VkResultToString(result));
			KBR_CORE_ASSERT(false, "Failed to allocate vertex buffer memory: {}", VulkanHelpers::VkResultToString(result));
		}

		/*VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(VulkanContext::Get().GetDevice(), m_Buffer, &memRequirements);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = Utils::FindMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		if (vkAllocateMemory(device, &allocInfo, nullptr, &m_BufferMemory) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate vertex buffer memory!");
		}

		vkBindBufferMemory(device, m_Buffer, m_BufferMemory, 0);

		void* data;
		vkMapMemory(device, m_BufferMemory, 0, bufferInfo.size, 0, &data);
		memcpy(data, vertices, bufferInfo.size);
		vkUnmapMemory(device, m_BufferMemory);*/
	}

	VulkanIndexBuffer::VulkanIndexBuffer(const uint32_t* indices, const uint32_t count)
		:	m_Count(count)
	{
		KBR_PROFILE_FUNCTION();

		const VkDevice device = VulkanContext::Get().GetDevice();
		const VkPhysicalDevice physicalDevice = VulkanContext::Get().GetPhysicalDevice();

		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = sizeof(uint32_t) * count;
		bufferInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		if (vkCreateBuffer(device, &bufferInfo, nullptr, &m_Buffer) != VK_SUCCESS)
		{
			KBR_CORE_ERROR("Failed to create index buffer!");
			return;
		}

		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(VulkanContext::Get().GetDevice(), m_Buffer, &memRequirements);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = VulkanHelpers::FindMemoryType(physicalDevice, memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		if (vkAllocateMemory(device, &allocInfo, nullptr, &m_BufferMemory) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to allocate index buffer memory!");
		}

		vkBindBufferMemory(device, m_Buffer, m_BufferMemory, 0);

		void* data;
		vkMapMemory(device, m_BufferMemory, 0, bufferInfo.size, 0, &data);
		memcpy(data, indices, bufferInfo.size);
		vkUnmapMemory(device, m_BufferMemory);
	}

	VulkanIndexBuffer::~VulkanIndexBuffer() 
	{
		KBR_PROFILE_FUNCTION();

		const VkDevice device = VulkanContext::Get().GetDevice();

		vkDeviceWaitIdle(device);

		vkDestroyBuffer(device, m_Buffer, nullptr);
		vkFreeMemory(device, m_BufferMemory, nullptr);
	}

	void VulkanIndexBuffer::Bind() const
	{
	}

	void VulkanIndexBuffer::Unbind() const
	{
	}

	uint32_t VulkanIndexBuffer::GetCount() const
	{
		return m_Count;
	}
}
