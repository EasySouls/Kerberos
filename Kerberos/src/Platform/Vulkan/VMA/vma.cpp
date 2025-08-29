#include "kbrpch.h"
#include "vma.h"

#include "Platform/Vulkan/VulkanHelpers.h"

namespace Kerberos::vma
{
	void Deleter::operator()(const VmaAllocator allocator) const noexcept
	{
		vmaDestroyAllocator(allocator);
	}

	Allocator CreateAllocator(const VkInstance instance, const VkPhysicalDevice physicalDevice, const VkDevice device) 
	{
		VmaAllocatorCreateInfo allocatorCi = VmaAllocatorCreateInfo{};
		allocatorCi.physicalDevice = physicalDevice;
		allocatorCi.device = device;
		allocatorCi.instance = instance;

		VmaAllocator allocator;
		if (const VkResult result = vmaCreateAllocator(&allocatorCi, &allocator); result != VK_SUCCESS)
		{
			KBR_CORE_ERROR("Failed to create vma allocator: {}", VulkanHelpers::VkResultToString(result));
			KBR_CORE_ASSERT(false, "Failed to create Vulkan Memory Allocator: {}", VulkanHelpers::VkResultToString(result));
			return {};
		}
		
		return allocator;
	}
}
