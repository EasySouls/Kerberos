#pragma once

#define GLFW_INCLUDE_VULKAN

#include "Kerberos/Renderer/GraphicsContext.h"
#include "VMA/vma.h"
#include "VulkanHelpers.h"

#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

#include "VulkanBuffer.h"

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
		std::vector<VkCommandBuffer> GetCommandBuffers() const { return m_CommandBuffers; }
		VkRenderPass GetRenderPass() const { return m_RenderPass; }
		VkPipeline GetPipeline() const { return m_GraphicsPipeline; }
		std::vector<VkFramebuffer> GetSwapChainFramebuffers() const { return m_SwapChainFramebuffers; }
		uint32_t GetCurrentFrameIndex() const { return m_CurrentFrame; }
		VkCommandPool GetCommandPool() const { return m_CommandPool; }
		const vma::Allocator& GetAllocator() const { return m_Allocator; }
		VkCommandBuffer GetCurrentCommandBuffer() const;

		VkDescriptorPool GetImGuiDescriptorPool() const { return m_ImGuiDescriptorPool; }

		/**
		* @brief Returns a one-time use command buffer that can be used to record commands.
		* The command buffer must be submitted using SubmitCommandBuffer() after recording.
		*/
		[[nodiscard]]
		VkCommandBuffer GetOneTimeCommandBuffer() const;

		/**
		* Submit a command buffer for execution.
		*/
		void SubmitCommandBuffer(VkCommandBuffer commandBuffer) const;

		static VulkanContext& Get() { return *s_Instance; }

	private:
		void RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex) const;

		void CreateInstance();
		void SetupDebugMessenger();
		void CreateSurface();
		void PickPhysicalDevice();
		void CreateLogicalDevice();
		void CreateVmaAllocator();
		void CreateSwapChain();
		void CreateImageViews();
		void CreateRenderPass();
		void CreateGraphicsPipeline();
		void CreateVertexBuffer();
		void CreateIndexBuffer();
		void CreateFramebuffers();
		void CreateCommandPool();
		void CreateCommandBuffers();
		void CreateSyncObjects();
		void CreateImGuiDescriptorPool();

		void CleanupSwapChain() const;
		void RecreateSwapChain();

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

		vma::Allocator m_Allocator{};

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
		std::vector<VkCommandBuffer> m_CommandBuffers;

		VkDescriptorPool m_ImGuiDescriptorPool = VK_NULL_HANDLE;

		std::vector<VkSemaphore> m_ImageAvailableSemaphores;
		std::vector<VkSemaphore> m_RenderFinishedSemaphores;
		std::vector<VkFence> m_InFlightFences;

		uint32_t m_CurrentFrame = 0;

		Scope<VertexBuffer> m_VertexBuffer;
		Scope<IndexBuffer> m_IndexBuffer;

		static VulkanContext* s_Instance;
	};
}
