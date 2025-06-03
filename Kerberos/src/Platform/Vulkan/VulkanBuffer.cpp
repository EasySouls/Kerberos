#include "kbrpch.h"
#include "VulkanBuffer.h"

#include "VulkanContext.h"

namespace Kerberos
{
	namespace Utils
	{
		static uint32_t FindMemoryType(const uint32_t typeFilter, const VkMemoryPropertyFlags properties)
		{
			const VkPhysicalDevice physicalDevice = VulkanContext::Get().GetPhysicalDevice();

			VkPhysicalDeviceMemoryProperties memProperties;
			vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);
			for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
			{
				if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
				{
					return i;
				}
			}
			KBR_CORE_ERROR("Failed to find suitable memory type!");
			return 0; // Should never reach here
		}
	}

	VulkanVertexBuffer::VulkanVertexBuffer(const float* vertices, const uint32_t size)
	{
		KBR_PROFILE_FUNCTION();

		const VkDevice device = VulkanContext::Get().GetDevice();

		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = sizeof(float) * size;
		bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		if (vkCreateBuffer(device, &bufferInfo, nullptr, &m_Buffer) != VK_SUCCESS)
		{
			KBR_CORE_ERROR("Failed to create vertex buffer!");
			return;
		}

		VkMemoryRequirements memRequirements;
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
		vkUnmapMemory(device, m_BufferMemory);
	}

	VulkanVertexBuffer::VulkanVertexBuffer(uint32_t size)
	{
	}

	VulkanVertexBuffer::~VulkanVertexBuffer()
	{
		KBR_PROFILE_FUNCTION();

		const VkDevice device = VulkanContext::Get().GetDevice();

		vkDeviceWaitIdle(device);

		vkDestroyBuffer(device, m_Buffer, nullptr);
		vkFreeMemory(device, m_BufferMemory, nullptr);
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

	VulkanIndexBuffer::VulkanIndexBuffer(const uint32_t* indices, const uint32_t count)
		:	m_Count(count)
	{
		KBR_PROFILE_FUNCTION();

		const VkDevice device = VulkanContext::Get().GetDevice();

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
		allocInfo.memoryTypeIndex = Utils::FindMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

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
