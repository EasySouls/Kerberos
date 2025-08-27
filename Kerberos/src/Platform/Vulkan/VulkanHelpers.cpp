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

	VkFormat VulkanHelpers::ShaderDataTypeToVulkanFormat(const ShaderDataType type)
	{
		/// TODO: Check if the formats are correct for Mat3 and Mat4

		switch (type)
		{
		case ShaderDataType::Float:		return VK_FORMAT_R32_SFLOAT;
		case ShaderDataType::Float2:	return VK_FORMAT_R32G32_SFLOAT;
		case ShaderDataType::Float3:	return VK_FORMAT_R32G32B32_SFLOAT;
		case ShaderDataType::Float4:	return VK_FORMAT_R32G32B32A32_SFLOAT;
		case ShaderDataType::Mat3:		return VK_FORMAT_R32G32B32_SFLOAT; // Treated as array of vec3
		case ShaderDataType::Mat4:		return VK_FORMAT_R32G32B32A32_SFLOAT; // Treated as array of vec4
		case ShaderDataType::Int:		return VK_FORMAT_R32_SINT;
		case ShaderDataType::Int2:		return VK_FORMAT_R32G32_SINT;
		case ShaderDataType::Int3:		return VK_FORMAT_R32G32B32_SINT;
		case ShaderDataType::Int4:		return VK_FORMAT_R32G32B32A32_SINT;
		case ShaderDataType::Bool:		return VK_FORMAT_R8_UINT; /// No direct boolean format, using uint8
		case ShaderDataType::None:		return VK_FORMAT_UNDEFINED;
		}

		KBR_CORE_ASSERT(false, "Unknown ShaderDataType!");
		return VK_FORMAT_UNDEFINED;
	}

	VkPrimitiveTopology VulkanHelpers::GetVulkanPrimitiveTopology(const Pipeline::Topology topology)
	{
		switch (topology)
		{
		case Pipeline::Topology::Triangles:	return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		case Pipeline::Topology::Lines:		return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
		case Pipeline::Topology::LineStrip:	return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
		}
		KBR_CORE_ASSERT(false, "Unknown Pipeline Topology!");
		return VK_PRIMITIVE_TOPOLOGY_MAX_ENUM;
	}

	VkCullModeFlagBits VulkanHelpers::GetVulkanCullMode(const Pipeline::CullMode cullMode)
	{
		switch (cullMode)
		{
		case Pipeline::CullMode::None:			return VK_CULL_MODE_NONE;
		case Pipeline::CullMode::Front:			return VK_CULL_MODE_FRONT_BIT;
		case Pipeline::CullMode::Back:			return VK_CULL_MODE_BACK_BIT;
		case Pipeline::CullMode::FrontAndBack:	return VK_CULL_MODE_FRONT_AND_BACK;
		}
		KBR_CORE_ASSERT(false, "Unknown Pipeline CullMode!");
		return VK_CULL_MODE_FLAG_BITS_MAX_ENUM;
	}


	VkFrontFace VulkanHelpers::GetVulkanFrontFace(const Pipeline::WindingOrder frontFace)
	{
		switch (frontFace)
		{
		case Pipeline::WindingOrder::Clockwise:		return VK_FRONT_FACE_CLOCKWISE;
		case Pipeline::WindingOrder::CounterClockwise: return VK_FRONT_FACE_COUNTER_CLOCKWISE;
		}
		KBR_CORE_ASSERT(false, "Unknown Pipeline WindingOrder!");
		return VK_FRONT_FACE_MAX_ENUM;
	}

	VkCompareOp VulkanHelpers::GetVulkanDepthCompareOp(const Pipeline::DepthTest depthTest)
	{
		switch (depthTest)
		{
		case Pipeline::DepthTest::None:			return VK_COMPARE_OP_NEVER;
		case Pipeline::DepthTest::Less:			return VK_COMPARE_OP_LESS;
		case Pipeline::DepthTest::LessEqual:	return VK_COMPARE_OP_LESS_OR_EQUAL;
		case Pipeline::DepthTest::Equal:		return VK_COMPARE_OP_EQUAL;
		case Pipeline::DepthTest::Greater:		return VK_COMPARE_OP_GREATER;
		case Pipeline::DepthTest::GreaterEqual:	return VK_COMPARE_OP_GREATER_OR_EQUAL;
		case Pipeline::DepthTest::NotEqual:		return VK_COMPARE_OP_NOT_EQUAL;
		case Pipeline::DepthTest::Always:		return VK_COMPARE_OP_ALWAYS;
		case Pipeline::DepthTest::Never:		return VK_COMPARE_OP_NEVER;
		}
		KBR_CORE_ASSERT(false, "Unknown Pipeline DepthTest!");
		return VK_COMPARE_OP_MAX_ENUM;
	}
}
