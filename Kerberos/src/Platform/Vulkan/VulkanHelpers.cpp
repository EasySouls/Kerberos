#include "kbrpch.h"
#include "VulkanHelpers.h"

namespace Kerberos
{
	uint32_t VulkanHelpers::FindMemoryType(const VkPhysicalDevice physicalDevice, const uint32_t typeFilter,
		const VkMemoryPropertyFlags properties)
	{
		VkPhysicalDeviceMemoryProperties memProperties;
		vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);
		for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
		{
			if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
			{
				return i;
			}
		}
		KBR_CORE_ASSERT(false, "Failed to find suitable memory type!");
		return 0xFFFFFFFF;
	}

	VkResult VulkanHelpers::CreateDebugUtilsMessengerEXT(const VkInstance instance,
		const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator,
		VkDebugUtilsMessengerEXT* pDebugMessenger) 
	{
		const auto func = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT"));
		if (func != nullptr)
		{
			return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
		}

		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}

	void VulkanHelpers::DestroyDebugUtilsMessengerEXT(const VkInstance instance,
		const VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) 
	{
		const auto func = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT"));
		if (func != nullptr)
		{
			func(instance, debugMessenger, pAllocator);
		}
	}

	void VulkanHelpers::SetObjectDebugName(const VkDevice device, const VkObjectType objectType, const uint64_t objectHandle, const std::string& name)
	{
		VkDebugUtilsObjectNameInfoEXT nameInfo{};
		nameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
		nameInfo.objectType = objectType;
		nameInfo.objectHandle = objectHandle;
		nameInfo.pObjectName = name.c_str();

		const auto func = reinterpret_cast<PFN_vkSetDebugUtilsObjectNameEXT>(vkGetDeviceProcAddr(device, "vkSetDebugUtilsObjectNameEXT"));

		if (func == nullptr)
		{
			KBR_CORE_WARN("Failed to load vkSetDebugUtilsObjectNameEXT - debug naming will be disabled");
			return;
		}

		func(device, &nameInfo);
	}

	bool VulkanHelpers::IsFormatSupported(const VkPhysicalDevice physicalDevice, const VkFormat format, const VkImageTiling tiling,
		const VkFormatFeatureFlags features) 
	{
		VkFormatProperties props;
		vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);

		if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features)
		{
			return true;
		}
		if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features)
		{
			return true;
		}

		return false;
	}
}
