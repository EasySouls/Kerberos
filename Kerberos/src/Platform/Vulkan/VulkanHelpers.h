#pragma once 

#include <vulkan/vulkan.h>
#include <optional>

namespace Kerberos
{
    struct QueueFamilyIndices
    {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;

        bool IsComplete() const
        {
            return graphicsFamily.has_value() && presentFamily.has_value();
        }
    };

	class VulkanHelpers
	{
	public:
        static uint32_t FindMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties);

        static VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);

        static void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);

        static void SetObjectDebugName(VkDevice device, VkObjectType objectType, uint64_t objectHandle, const std::string& name);

		static const char* VkResultToString(const VkResult result)
		{
            switch (result)
            {
            case VK_SUCCESS:
                return "VK_SUCCESS";
            case VK_NOT_READY:
                return "VK_NOT_READY";
            case VK_TIMEOUT:
                return "VK_TIMEOUT";
            case VK_EVENT_SET:
                return "VK_EVENT_SET";
            case VK_EVENT_RESET:
                return "VK_EVENT_RESET";
            case VK_INCOMPLETE:
                return "VK_INCOMPLETE";
            case VK_ERROR_OUT_OF_HOST_MEMORY:
                return "VK_ERROR_OUT_OF_HOST_MEMORY";
            case VK_ERROR_OUT_OF_DEVICE_MEMORY:
                return "VK_ERROR_OUT_OF_DEVICE_MEMORY";
            case VK_ERROR_INITIALIZATION_FAILED:
                return "VK_ERROR_INITIALIZATION_FAILED";
            case VK_ERROR_DEVICE_LOST:
                return "VK_ERROR_DEVICE_LOST";
            case VK_ERROR_MEMORY_MAP_FAILED:
                return "VK_ERROR_MEMORY_MAP_FAILED";
            case VK_ERROR_LAYER_NOT_PRESENT:
                return "VK_ERROR_LAYER_NOT_PRESENT";
            case VK_ERROR_EXTENSION_NOT_PRESENT:
                return "VK_ERROR_EXTENSION_NOT_PRESENT";
            case VK_ERROR_FEATURE_NOT_PRESENT:
                return "VK_ERROR_FEATURE_NOT_PRESENT";
            case VK_ERROR_INCOMPATIBLE_DRIVER:
                return "VK_ERROR_INCOMPATIBLE_DRIVER";
            case VK_ERROR_TOO_MANY_OBJECTS:
                return "VK_ERROR_TOO_MANY_OBJECTS";
            case VK_ERROR_FORMAT_NOT_SUPPORTED:
                return "VK_ERROR_FORMAT_NOT_SUPPORTED";
            case VK_ERROR_FRAGMENTED_POOL:
                return "VK_ERROR_FRAGMENTED_POOL";
            case VK_ERROR_UNKNOWN:
                return "VK_ERROR_UNKNOWN";
            case VK_ERROR_OUT_OF_POOL_MEMORY:
                return "VK_ERROR_OUT_OF_POOL_MEMORY";
            case VK_ERROR_INVALID_EXTERNAL_HANDLE:
                return "VK_ERROR_INVALID_EXTERNAL_HANDLE";
            case VK_ERROR_FRAGMENTATION:
                return "VK_ERROR_FRAGMENTATION";
            case VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS:
                return "VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS";
            case VK_ERROR_SURFACE_LOST_KHR:
                return "VK_ERROR_SURFACE_LOST_KHR";
            case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:
                return "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR";
            case VK_SUBOPTIMAL_KHR:
                return "VK_SUBOPTIMAL_KHR";
            case VK_ERROR_OUT_OF_DATE_KHR:
                return "VK_ERROR_OUT_OF_DATE_KHR";
            case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR:
                return "VK_ERROR_INCOMPATIBLE_DISPLAY_KHR";
            case VK_ERROR_VALIDATION_FAILED_EXT:
                return "VK_ERROR_VALIDATION_FAILED_EXT";
            case VK_ERROR_INVALID_SHADER_NV:
                return "VK_ERROR_INVALID_SHADER_NV";
            case VK_ERROR_NOT_PERMITTED_EXT:
                return "VK_ERROR_NOT_PERMITTED_EXT";
            case VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT:
                return "VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT";
            case VK_THREAD_IDLE_KHR:
                return "VK_THREAD_IDLE_KHR";
            case VK_THREAD_DONE_KHR:
                return "VK_THREAD_DONE_KHR";
            case VK_OPERATION_DEFERRED_KHR:
                return "VK_OPERATION_DEFERRED_KHR";
            case VK_OPERATION_NOT_DEFERRED_KHR:
                return "VK_OPERATION_NOT_DEFERRED_KHR";
            case VK_PIPELINE_COMPILE_REQUIRED_EXT:
                return "VK_PIPELINE_COMPILE_REQUIRED_EXT";
            case VK_ERROR_IMAGE_USAGE_NOT_SUPPORTED_KHR:
				return "VK_ERROR_IMAGE_USAGE_NOT_SUPPORTED_KHR";
            case VK_ERROR_VIDEO_PICTURE_LAYOUT_NOT_SUPPORTED_KHR:
	            return "VK_ERROR_VIDEO_PICTURE_LAYOUT_NOT_SUPPORTED_KHR";
            case VK_ERROR_VIDEO_PROFILE_OPERATION_NOT_SUPPORTED_KHR:
				return "VK_ERROR_VIDEO_PROFILE_OPERATION_NOT_SUPPORTED_KHR";
            case VK_ERROR_VIDEO_PROFILE_FORMAT_NOT_SUPPORTED_KHR:
				return "VK_ERROR_VIDEO_PROFILE_FORMAT_NOT_SUPPORTED_KHR";
            case VK_ERROR_VIDEO_PROFILE_CODEC_NOT_SUPPORTED_KHR:
				return "VK_ERROR_VIDEO_PROFILE_CODEC_NOT_SUPPORTED_KHR";
            case VK_ERROR_VIDEO_STD_VERSION_NOT_SUPPORTED_KHR:
				return "VK_ERROR_VIDEO_STD_VERSION_NOT_SUPPORTED_KHR";
            case VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT:
				return "VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT";
            case VK_ERROR_INVALID_VIDEO_STD_PARAMETERS_KHR:
				return "VK_ERROR_INVALID_VIDEO_STD_PARAMETERS_KHR";
            case VK_ERROR_COMPRESSION_EXHAUSTED_EXT:
				return "VK_ERROR_COMPRESSION_EXHAUSTED_EXT";
            case VK_INCOMPATIBLE_SHADER_BINARY_EXT:
				return "VK_INCOMPATIBLE_SHADER_BINARY_EXT";
            case VK_PIPELINE_BINARY_MISSING_KHR:
				return "VK_PIPELINE_BINARY_MISSING_KHR";
            case VK_ERROR_NOT_ENOUGH_SPACE_KHR:
				return "VK_ERROR_NOT_ENOUGH_SPACE_KHR";
            case VK_RESULT_MAX_ENUM:
				return "VK_RESULT_MAX_ENUM";
            default:
                return "UNKNOWN_ERROR";
            }
		}

        static void ImGuiVulkanCheckResult(const VkResult err)
        {
            if (err == VK_SUCCESS) return;

            KBR_CORE_ERROR("ImGui Vulkan Error: {0}", VulkanHelpers::VkResultToString(err));
            KBR_CORE_ASSERT(false, "ImGui Vulkan Error: {0}", VulkanHelpers::VkResultToString(err));
        }
	};
}