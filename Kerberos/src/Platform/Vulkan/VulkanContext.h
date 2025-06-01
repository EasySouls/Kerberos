#pragma once

#define GLFW_INCLUDE_VULKAN

#include "Kerberos/Renderer/GraphicsContext.h"
#include "VulkanHelpers.h"

#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>


namespace Kerberos
{
	class VulkanContext : public GraphicsContext
	{
	public:
		struct SwapChainSupportDetails
		{
			VkSurfaceCapabilitiesKHR capabilities;
			std::vector<VkSurfaceFormatKHR> formats;
			std::vector<VkPresentModeKHR> presentModes;
		};

		explicit VulkanContext(GLFWwindow* windowHandle);
		~VulkanContext() override;

		void Init() override;
		void SwapBuffers() override;

		QueueFamilyIndices FindQueueFamilies() const;

		VkInstance GetInstance() const { return m_Instance; }
		VkSurfaceKHR GetSurface() const { return m_Surface; }
		VkDevice GetDevice() const { return m_Device; }
		VkPhysicalDevice GetPhysicalDevice() const { return m_PhysicalDevice; }
		VkFormat GetSwapChainImageFormat() const { return m_SwapChainImageFormat; }
		VkExtent2D GetSwapChainExtent() const { return m_SwapChainExtent; }
		const std::vector<VkImage>& GetSwapChainImages() const { return m_SwapChainImages; }
		const std::vector<VkImageView>& GetSwapChainImageViews() const { return m_SwapChainImageViews; }
		VkQueue GetGraphicsQueue() const { return m_GraphicsQueue; }
		VkQueue GetPresentQueue() const { return m_PresentQueue; }
		uint32_t GetGraphicsQueueFamilyIndex() const { return m_GraphicsQueueFamilyIndex; }
		uint32_t GetPresentQueueFamilyIndex() const { return m_PresentQueueFamilyIndex; }
		VkCommandBuffer GetCommandBuffer() const { return m_CommandBuffer; }
		VkRenderPass GetRenderPass() const { return m_RenderPass; }
		VkPipeline GetPipeline() const { return m_GraphicsPipeline; }
		std::vector<VkFramebuffer> GetSwapChainFramebuffers() const { return m_SwapChainFramebuffers; }

		static VulkanContext& Get() { return *s_Instance; }

	private:
		void CreateInstance();
		void SetupDebugMessenger();
		void CreateSurface();
		void PickPhysicalDevice();
		void CreateLogicalDevice();
		void CreateSwapChain();
		void CreateImageViews();
		void CreateRenderPass();
		void CreateGraphicsPipeline();
		void CreateFramebuffers();
		void CreateCommandPool();
		void CreateCommandBuffer();

		/////////////////////////////////////////////////////////
		//////////////////// Helper methods  ////////////////////
		/////////////////////////////////////////////////////////
		
		static std::vector<const char*> GetRequiredExtensions();
		static bool CheckValidationLayerSupport();
		static void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
		bool IsDeviceSuitable(VkPhysicalDevice device) const;
		static bool CheckDeviceExtensionSupport(VkPhysicalDevice device);
		QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device) const;
		SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device) const;

		static VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
		static VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
		VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) const;

	private:
		GLFWwindow* m_WindowHandle;

		VkInstance m_Instance = VK_NULL_HANDLE;
		VkDebugUtilsMessengerEXT m_DebugMessenger = VK_NULL_HANDLE;
		VkSurfaceKHR m_Surface = VK_NULL_HANDLE;
		VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;
		VkDevice m_Device = VK_NULL_HANDLE;

		VkQueue m_GraphicsQueue = VK_NULL_HANDLE;
		VkQueue m_PresentQueue = VK_NULL_HANDLE;
		uint32_t m_GraphicsQueueFamilyIndex = UINT32_MAX;
		uint32_t m_PresentQueueFamilyIndex = UINT32_MAX;

		VkSwapchainKHR m_SwapChain = VK_NULL_HANDLE;
		std::vector<VkImage> m_SwapChainImages;
		VkFormat m_SwapChainImageFormat;
		VkExtent2D m_SwapChainExtent;
		std::vector<VkImageView> m_SwapChainImageViews;

		VkRenderPass m_RenderPass = VK_NULL_HANDLE;
		VkPipelineLayout m_PipelineLayout = VK_NULL_HANDLE;
		VkPipeline m_GraphicsPipeline = VK_NULL_HANDLE;
		std::vector<VkFramebuffer> m_SwapChainFramebuffers;
		VkCommandPool m_CommandPool = VK_NULL_HANDLE;
		VkCommandBuffer m_CommandBuffer = VK_NULL_HANDLE;

		static VulkanContext* s_Instance;
	};
}
