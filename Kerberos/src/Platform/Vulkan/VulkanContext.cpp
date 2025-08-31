#include "kbrpch.h"

#include "VulkanContext.h"
#include "Kerberos/Core.h"

#include <cstring>
#include <backends/imgui_impl_vulkan.h>

#include "imgui.h"
#include "VulkanShader.h"

constexpr int maxFramesInFlight = 2;

const std::vector<const char*> validationLayers = {
	"VK_LAYER_KHRONOS_validation"
};

const std::vector<const char*> deviceExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME,

};

#ifdef KBR_DEBUG
constexpr bool enableValidationLayers = true;
#else
constexpr bool enableValidationLayers = false;
#endif

static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
	const VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData)
{
	if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
	{
		KBR_CORE_ERROR("Validation layer: {0} - {1}: {2}", pCallbackData->messageIdNumber, pCallbackData->pMessageIdName, pCallbackData->pMessage);
	}
	else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
	{
		KBR_CORE_WARN("Validation layer: {0} - {1}: {2}", pCallbackData->messageIdNumber, pCallbackData->pMessageIdName, pCallbackData->pMessage);
	}
	else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
	{
		KBR_CORE_INFO("Validation layer: {0} - {1}: {2}", pCallbackData->messageIdNumber, pCallbackData->pMessageIdName, pCallbackData->pMessage);
	}
	else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT)
	{
		KBR_CORE_TRACE("Validation layer: {0} - {1}: {2}", pCallbackData->messageIdNumber, pCallbackData->pMessageIdName, pCallbackData->pMessage);
	}

	return VK_FALSE;
}

namespace Kerberos
{
	VulkanContext* VulkanContext::s_Instance = nullptr;

	VulkanContext::VulkanContext(GLFWwindow* windowHandle)
		: m_WindowHandle(windowHandle)
	{
		KBR_CORE_ASSERT(m_WindowHandle, "Window handle is null!");

		KBR_CORE_ASSERT(!s_Instance, "VulkanContext already exists!");
		s_Instance = this;
	}

	VulkanContext::~VulkanContext()
	{
		vkDeviceWaitIdle(m_Device);

		CleanupSwapChain();

		vkDestroyPipeline(m_Device, m_GraphicsPipeline, nullptr);
		vkDestroyPipelineLayout(m_Device, m_PipelineLayout, nullptr);

		vkDestroyRenderPass(m_Device, m_RenderPass, nullptr);

		for (size_t i = 0; i < maxFramesInFlight; ++i)
		{
			vkDestroySemaphore(m_Device, m_ImageAvailableSemaphores[i], nullptr);
			vkDestroySemaphore(m_Device, m_RenderFinishedSemaphores[i], nullptr);
			vkDestroyFence(m_Device, m_InFlightFences[i], nullptr);
		}

		vkDestroyCommandPool(m_Device, m_CommandPool, nullptr);

		vkDestroyDevice(m_Device, nullptr);

		if (enableValidationLayers)
		{
			VulkanHelpers::DestroyDebugUtilsMessengerEXT(m_Instance, m_DebugMessenger, nullptr);
		}

		vkDestroySurfaceKHR(m_Instance, m_Surface, nullptr);
		vkDestroyInstance(m_Instance, nullptr);
	}

	void VulkanContext::Init()
	{
		CreateInstance();
		SetupDebugMessenger();
		CreateSurface();
		PickPhysicalDevice();
		CreateLogicalDevice();
		CreateVmaAllocator();
		CreateSwapChain();
		CreateImageViews();
		CreateRenderPass();
		CreateGraphicsPipeline();
		CreateVertexBuffer();
		CreateIndexBuffer();
		CreateFramebuffers();
		CreateCommandPool();
		CreateCommandBuffers();
		CreateSyncObjects();
		CreateImGuiDescriptorPool();
	}

	void VulkanContext::SwapBuffers()
	{
		/// Wait for the fence to be signaled, then reset it
		vkWaitForFences(m_Device, 1, &m_InFlightFences[m_CurrentFrame], VK_TRUE, UINT64_MAX);

		uint32_t imageIndex;
		if (const VkResult result = vkAcquireNextImageKHR(m_Device, m_SwapChain, UINT64_MAX, m_ImageAvailableSemaphores[m_CurrentFrame], VK_NULL_HANDLE, &imageIndex); result != VK_SUCCESS)
		{
			if (result == VK_ERROR_OUT_OF_DATE_KHR)
			{
				KBR_CORE_WARN("Swap chain out of date, recreating swap chain...");
				RecreateSwapChain();
				return;
			}
			if (result != VK_SUBOPTIMAL_KHR)
			{
				KBR_CORE_ERROR("Failed to acquire swap chain image! Result: {0}", VulkanHelpers::VkResultToString(result));
				throw std::runtime_error("failed to acquire swap chain image!");
			}
		}

		/// We only reset the fence here, because if we return early due to an out of date swap chain,
		/// and the fence stays signaled, we might get into a deadlock
		vkResetFences(m_Device, 1, &m_InFlightFences[m_CurrentFrame]);

		vkResetCommandBuffer(m_CommandBuffers[m_CurrentFrame], 0);

		RecordCommandBuffer(m_CommandBuffers[m_CurrentFrame], imageIndex);

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		const VkSemaphore waitSemaphores[] = { m_ImageAvailableSemaphores[m_CurrentFrame] };
		constexpr VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;

		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &m_CommandBuffers[m_CurrentFrame];

		const VkSemaphore signalSemaphores[] = { m_RenderFinishedSemaphores[m_CurrentFrame] };
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		if (const VkResult result = vkQueueSubmit(m_GraphicsQueue, 1, &submitInfo, m_InFlightFences[m_CurrentFrame]); result != VK_SUCCESS) {
			KBR_CORE_ERROR("Failed to submit draw command buffer! Result: {0}", VulkanHelpers::VkResultToString(result));
			KBR_CORE_ASSERT(false, "Failed to submit draw command buffer!");
			throw std::runtime_error("failed to submit draw command buffer!");
		}

		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphores;

		const VkSwapchainKHR swapChains[] = { m_SwapChain };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;
		presentInfo.pImageIndices = &imageIndex;

		if (const VkResult result = vkQueuePresentKHR(m_PresentQueue, &presentInfo); result != VK_SUCCESS)
		{
			if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
			{
				RecreateSwapChain();
			}
			else 
			{
				KBR_CORE_ERROR("Failed to present swapchain image! Result: {0}", VulkanHelpers::VkResultToString(result));
				KBR_CORE_ASSERT(false, "Failed to present swapchain image!");
				throw std::runtime_error("failed to present swap chain image!");
			}
		}

		const ImGuiIO& io = ImGui::GetIO();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			//GLFWwindow* backupCurrentContext = glfwGetCurrentContext();
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
			//glfwMakeContextCurrent(backupCurrentContext);
		}

		m_CurrentFrame = (m_CurrentFrame + 1) % maxFramesInFlight;
	}

	void VulkanContext::CreateImGuiDescriptorPool()
	{
		// These pool sizes are typical for ImGui and should be sufficient.
		const VkDescriptorPoolSize poolSizes[] =
		{
			{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
			{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
			{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
			{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
		};
		VkDescriptorPoolCreateInfo poolInfo = {};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
		poolInfo.maxSets = 1000 * IM_ARRAYSIZE(poolSizes);
		poolInfo.poolSizeCount = static_cast<uint32_t>(IM_ARRAYSIZE(poolSizes));
		poolInfo.pPoolSizes = poolSizes;

		if (vkCreateDescriptorPool(m_Device, &poolInfo, nullptr, &m_ImGuiDescriptorPool) != VK_SUCCESS)
		{
			KBR_CORE_ASSERT(false, "Failed to create ImGui descriptor pool!");
			throw std::runtime_error("failed to create ImGui descriptor pool!");
		}
	}

	void VulkanContext::CleanupSwapChain() const 
	{
		for (const auto framebuffer : m_SwapChainFramebuffers)
		{
			vkDestroyFramebuffer(m_Device, framebuffer, nullptr);
		}

		for (const auto imageView : m_SwapChainImageViews)
		{
			vkDestroyImageView(m_Device, imageView, nullptr);
		}

		vkDestroySwapchainKHR(m_Device, m_SwapChain, nullptr);
	}

	void VulkanContext::RecreateSwapChain() 
	{
		int width = 0, height = 0;
		glfwGetFramebufferSize(m_WindowHandle, &width, &height);
		while (width == 0 || height == 0)
		{
			glfwGetFramebufferSize(m_WindowHandle, &width, &height);
			glfwWaitEvents();
		}

		vkDeviceWaitIdle(m_Device);

		CleanupSwapChain();

		CreateSwapChain();
		CreateImageViews();
		CreateFramebuffers();
	}

	void VulkanContext::RecordCommandBuffer(const VkCommandBuffer commandBuffer, const uint32_t imageIndex) const
	{
		constexpr VkCommandBufferBeginInfo beginInfo{
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
			.pNext = nullptr,
		};

		if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to begin recording command buffer!");
		}

		VkClearValue clearColor = { {{0.0f, 0.0f, 0.0f, 1.0f}} };

		const VkRenderPassBeginInfo renderPassInfo{
			.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
			.pNext = nullptr,
			.renderPass = m_RenderPass,
			.framebuffer = m_SwapChainFramebuffers[imageIndex],
			.renderArea = {
				.offset = { 0, 0 },
				.extent = m_SwapChainExtent
			},
			.clearValueCount = 1,
			.pClearValues = &clearColor
		};

		vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_GraphicsPipeline);

		VkViewport viewport;
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(m_SwapChainExtent.width);
		viewport.height = static_cast<float>(m_SwapChainExtent.height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

		VkRect2D scissor;
		scissor.offset = {.x = 0, .y = 0 };
		scissor.extent = m_SwapChainExtent;
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

		const auto& vertexBuffer = m_VertexBuffer->As<VulkanVertexBuffer>();

		const VkBuffer vertexBuffers[] = { vertexBuffer.GetVkBuffer() };
		constexpr VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

		const auto& indexBuffer = m_IndexBuffer->As<VulkanIndexBuffer>();

		vkCmdBindIndexBuffer(commandBuffer, indexBuffer.GetVkBuffer(), 0, static_cast<VkIndexType>(indexBuffer.GetType()));

		//vkCmdDraw(commandBuffer, 32, 1, 0, 0);

		vkCmdDrawIndexed(commandBuffer, m_IndexBuffer->GetCount(), 1, 0, 0, 0);

		ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);

		vkCmdEndRenderPass(commandBuffer);

		if (const VkResult result = vkEndCommandBuffer(commandBuffer); result != VK_SUCCESS)
		{
			KBR_CORE_ASSERT(false, "Failed to record command buffer! Result: {0}", VulkanHelpers::VkResultToString(result));
			throw std::runtime_error("failed to record command buffer!");
		}
	}

	void VulkanContext::CreateInstance()
	{
		if (enableValidationLayers && !CheckValidationLayerSupport())
		{
			throw std::runtime_error("validation layers requested, but not available!");
		}

		VkApplicationInfo appInfo{};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "Kerberos";
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "Kerberos Engine";
		appInfo.engineVersion = VK_MAKE_API_VERSION(1, 0, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_4;

		VkInstanceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;

		const auto extensions = GetRequiredExtensions();
		createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
		createInfo.ppEnabledExtensionNames = extensions.data();

		VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
		if (enableValidationLayers)
		{
			createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
			createInfo.ppEnabledLayerNames = validationLayers.data();

			PopulateDebugMessengerCreateInfo(debugCreateInfo);
			createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
		}
		else
		{
			createInfo.enabledLayerCount = 0;

			createInfo.pNext = nullptr;
		}

		if (const VkResult result = vkCreateInstance(&createInfo, nullptr, &m_Instance); result != VK_SUCCESS)
		{
			KBR_CORE_ASSERT(false, "Failed to create instance! Result: {0}", VulkanHelpers::VkResultToString(result));
			throw std::runtime_error("failed to create instance!");
		}
	}

	void VulkanContext::SetupDebugMessenger()
	{
		if (!enableValidationLayers) return;

		VkDebugUtilsMessengerCreateInfoEXT createInfo;
		PopulateDebugMessengerCreateInfo(createInfo);

		if (VulkanHelpers::CreateDebugUtilsMessengerEXT(m_Instance, &createInfo, nullptr, &m_DebugMessenger) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to set up debug messenger!");
		}
	}

	void VulkanContext::CreateSurface()
	{
		if (const VkResult result = glfwCreateWindowSurface(m_Instance, m_WindowHandle, nullptr, &m_Surface); result != VK_SUCCESS)
		{
			KBR_CORE_ASSERT(false, "Failed to create window surface! Result: {0}", VulkanHelpers::VkResultToString(result));
			throw std::runtime_error("failed to create window surface!");
		}
	}

	void VulkanContext::PickPhysicalDevice()
	{
		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(m_Instance, &deviceCount, nullptr);
		if (deviceCount == 0)
		{
			throw std::runtime_error("failed to find GPUs with Vulkan support!");
		}

		std::vector<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(m_Instance, &deviceCount, devices.data());

		for (const auto& device : devices)
		{
			if (IsDeviceSuitable(device))
			{
				m_PhysicalDevice = device;
				break;
			}
		}

		if (m_PhysicalDevice == VK_NULL_HANDLE)
		{
			throw std::runtime_error("failed to find a suitable GPU!");
		}

		/// List the properties of the physical device
		VkPhysicalDeviceProperties deviceProperties;
		vkGetPhysicalDeviceProperties(m_PhysicalDevice, &deviceProperties);

		const char* deviceType;
		switch (deviceProperties.deviceType)
		{
		case VK_PHYSICAL_DEVICE_TYPE_OTHER:
			deviceType = "Other";
			break;
		case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
			deviceType = "Integrated GPU";
			break;
		case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
			deviceType = "Discrete GPU";
			break;
		case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
			deviceType = "Virtual GPU";
			break;
		case VK_PHYSICAL_DEVICE_TYPE_CPU:
			deviceType = "CPU";
			break;
		case VK_PHYSICAL_DEVICE_TYPE_MAX_ENUM:
			KBR_CORE_ASSERT(false, "Invalid device type!");
			break;
		}

		KBR_CORE_INFO("Vulkan Physical Device Properties:");
		KBR_CORE_INFO("\tVulkan Device: {0}", deviceProperties.deviceName);
		KBR_CORE_INFO("\tVulkan Device Type: {0}", deviceType);
		KBR_CORE_INFO("\tVulkan Driver Version: {0}", deviceProperties.driverVersion);
		KBR_CORE_INFO("\tVulkan API Version: {0}.{1}.{2}", VK_VERSION_MAJOR(deviceProperties.apiVersion), VK_VERSION_MINOR(deviceProperties.apiVersion), VK_VERSION_PATCH(deviceProperties.apiVersion));
	}

	void VulkanContext::CreateLogicalDevice()
	{
		auto [graphicsFamily, presentFamily] = FindQueueFamilies(m_PhysicalDevice);

		if (!graphicsFamily.has_value() || !presentFamily.has_value())
		{
			throw std::runtime_error("Queue family indices are not complete!");
		}

		m_GraphicsQueueFamilyIndex = graphicsFamily.value();
		m_PresentQueueFamilyIndex = presentFamily.value();

		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
		std::set<uint32_t> uniqueQueueFamilies = { graphicsFamily.value(), presentFamily.value() };

		float queuePriority = 1.0f;
		for (uint32_t queueFamily : uniqueQueueFamilies)
		{
			VkDeviceQueueCreateInfo queueCreateInfo{};
			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfo.queueFamilyIndex = queueFamily;
			queueCreateInfo.queueCount = 1;
			queueCreateInfo.pQueuePriorities = &queuePriority;
			queueCreateInfos.push_back(queueCreateInfo);
		}

		/// Query the device features
		VkPhysicalDeviceFeatures deviceFeatures{};
		vkGetPhysicalDeviceFeatures(m_PhysicalDevice, &deviceFeatures);

		VkDeviceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

		createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
		createInfo.pQueueCreateInfos = queueCreateInfos.data();

		createInfo.pEnabledFeatures = &deviceFeatures;

		createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
		createInfo.ppEnabledExtensionNames = deviceExtensions.data();

		if (enableValidationLayers)
		{
			createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
			createInfo.ppEnabledLayerNames = validationLayers.data();
		}
		else
		{
			createInfo.enabledLayerCount = 0;
		}

		if (const VkResult result = vkCreateDevice(m_PhysicalDevice, &createInfo, nullptr, &m_Device); result != VK_SUCCESS)
		{
			KBR_CORE_ASSERT(false, "Failed to create logical device! Result: {0}", VulkanHelpers::VkResultToString(result));
			throw std::runtime_error("failed to create logical device!");
		}

		vkGetDeviceQueue(m_Device, graphicsFamily.value(), 0, &m_GraphicsQueue);
		vkGetDeviceQueue(m_Device, presentFamily.value(), 0, &m_PresentQueue);
	}

	void VulkanContext::CreateVmaAllocator() 
	{
		KBR_CORE_ASSERT(m_Instance != VK_NULL_HANDLE, "VkInstance has to be initialized to create allocator!");
		KBR_CORE_ASSERT(m_PhysicalDevice != VK_NULL_HANDLE, "VkPhysicalDevice has to be initialized to create allocator!");
		KBR_CORE_ASSERT(m_Device != VK_NULL_HANDLE, "VkDevice has to be initialized to create allocator!");

		m_Allocator = vma::CreateAllocator(m_Instance, m_PhysicalDevice, m_Device);
	}

	void VulkanContext::CreateSwapChain()
	{
		const SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(m_PhysicalDevice);

		const VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(swapChainSupport.formats);
		const VkPresentModeKHR presentMode = ChooseSwapPresentMode(swapChainSupport.presentModes);
		const VkExtent2D extent = ChooseSwapExtent(swapChainSupport.capabilities);

		uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
		if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
		{
			imageCount = swapChainSupport.capabilities.maxImageCount;
		}

		VkSwapchainCreateInfoKHR createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = m_Surface;
		createInfo.minImageCount = imageCount;
		createInfo.imageFormat = surfaceFormat.format;
		createInfo.imageColorSpace = surfaceFormat.colorSpace;
		createInfo.imageExtent = extent;
		createInfo.imageArrayLayers = 1;
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

		const auto [graphicsFamily, presentFamily] = FindQueueFamilies(m_PhysicalDevice);
		if (!graphicsFamily.has_value() || !presentFamily.has_value())
		{
			throw std::runtime_error("Queue family indices are not complete!");
		}
		const uint32_t queueFamilyIndices[] = { graphicsFamily.value(), presentFamily.value() };

		if (graphicsFamily != presentFamily)
		{
			createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			createInfo.queueFamilyIndexCount = 2;
			createInfo.pQueueFamilyIndices = queueFamilyIndices;
		}
		else
		{
			createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			createInfo.queueFamilyIndexCount = 0; // Optional
			createInfo.pQueueFamilyIndices = nullptr; // Optional
		}

		createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		createInfo.presentMode = presentMode;
		createInfo.clipped = VK_TRUE;
		createInfo.oldSwapchain = VK_NULL_HANDLE;

		if (const VkResult result = vkCreateSwapchainKHR(m_Device, &createInfo, nullptr, &m_SwapChain); result != VK_SUCCESS)
		{
			KBR_CORE_ASSERT(false, "Failed to create swapchain! Result: {0}", VulkanHelpers::VkResultToString(result));
			throw std::runtime_error("failed to create swapchain!");
		}

		vkGetSwapchainImagesKHR(m_Device, m_SwapChain, &imageCount, nullptr);
		m_SwapChainImages.resize(imageCount);
		vkGetSwapchainImagesKHR(m_Device, m_SwapChain, &imageCount, m_SwapChainImages.data());

		m_SwapChainImageFormat = surfaceFormat.format;
		m_SwapChainExtent = extent;
	}

	void VulkanContext::CreateImageViews()
	{
		m_SwapChainImageViews.resize(m_SwapChainImages.size());

		for (size_t i = 0; i < m_SwapChainImages.size(); i++)
		{
			VkImageViewCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			createInfo.image = m_SwapChainImages[i];
			createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			createInfo.format = m_SwapChainImageFormat;

			createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

			createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			createInfo.subresourceRange.baseMipLevel = 0;
			createInfo.subresourceRange.levelCount = 1;
			createInfo.subresourceRange.baseArrayLayer = 0;
			createInfo.subresourceRange.layerCount = 1;

			if (const VkResult result = vkCreateImageView(m_Device, &createInfo, nullptr, &m_SwapChainImageViews[i]); result != VK_SUCCESS)
			{
				KBR_CORE_ASSERT(false, "Failed to create image views! Result: {0}", VulkanHelpers::VkResultToString(result));
				throw std::runtime_error("failed to create image views!");
			}
		}
	}

	void VulkanContext::CreateRenderPass()
	{
		VkAttachmentDescription colorAttachment{};
		colorAttachment.format = m_SwapChainImageFormat;
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentReference colorAttachmentRef;
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;

		/// This makes sure that the render pass waits for the swap chain to be ready before starting to render
		VkSubpassDependency dependency{};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		VkRenderPassCreateInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = 1;
		renderPassInfo.pAttachments = &colorAttachment;
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;
		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &dependency;

		if (const VkResult result = vkCreateRenderPass(m_Device, &renderPassInfo, nullptr, &m_RenderPass); result != VK_SUCCESS)
		{
			KBR_CORE_ASSERT(false, "Failed to create render pass! Result: {0}", VulkanHelpers::VkResultToString(result));
			throw std::runtime_error("failed to create render pass!");
		}
	}

	void VulkanContext::CreateGraphicsPipeline()
	{
		VulkanShader basic3DShader("assets/shaders/shader3d-basic-vulkan.glsl");
		const auto& createShaderStages = basic3DShader.GetPipelineShaderStageCreateInfos();

		// TODO: Somehow we should check all the used shaders and their stages before creating the pipeline

		VkPipelineShaderStageCreateInfo shaderStages[2] = {};
		shaderStages[0] = createShaderStages.at(VK_SHADER_STAGE_VERTEX_BIT);
		shaderStages[1] = createShaderStages.at(VK_SHADER_STAGE_FRAGMENT_BIT);

		std::vector<VkDynamicState> dynamicStateEnables = {
			VK_DYNAMIC_STATE_VIEWPORT,
			VK_DYNAMIC_STATE_SCISSOR
		};

		VkPipelineDynamicStateCreateInfo dynamicState = {};
		dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStateEnables.size());
		dynamicState.pDynamicStates = dynamicStateEnables.data();

		const std::vector<VkVertexInputBindingDescription>& bindingDescriptions = basic3DShader.GetVertexInputBindingDescriptions();
		const std::vector<VkVertexInputAttributeDescription>& attributeDescriptions = basic3DShader.GetVertexInputAttributeDescriptions();

		VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(bindingDescriptions.size());
		vertexInputInfo.pVertexBindingDescriptions = bindingDescriptions.data();
		vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
		vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

		VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
		inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssembly.primitiveRestartEnable = VK_FALSE;

		// The viewport and scissor can be dynamically set in the command buffer
		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(m_SwapChainExtent.width);
		viewport.height = static_cast<float>(m_SwapChainExtent.height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		VkRect2D scissor{};
		scissor.offset = { .x = 0, .y = 0 };
		scissor.extent = m_SwapChainExtent;

		VkPipelineViewportStateCreateInfo viewportState{};
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount = 1;
		viewportState.pViewports = &viewport;
		viewportState.scissorCount = 1;
		viewportState.pScissors = &scissor;

		// It performs depth testing, face culling and the scissor test, and it can be configured 
		// to output fragments that fill entire polygons or just the edges (wireframe rendering).
		VkPipelineRasterizationStateCreateInfo rasterizer{};
		rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizer.depthClampEnable = VK_FALSE;
		rasterizer.rasterizerDiscardEnable = VK_FALSE;
		rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizer.lineWidth = 1.0f;
		rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
		rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
		rasterizer.depthBiasEnable = VK_FALSE;
		rasterizer.depthBiasConstantFactor = 0.0f; // Optional
		rasterizer.depthBiasClamp = 0.0f; // Optional
		rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

		VkPipelineMultisampleStateCreateInfo multisampling{};
		multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampling.sampleShadingEnable = VK_FALSE;
		multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		multisampling.minSampleShading = 1.0f; // Optional
		multisampling.pSampleMask = nullptr; // Optional
		multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
		multisampling.alphaToOneEnable = VK_FALSE; // Optional

		VkPipelineColorBlendAttachmentState colorBlendAttachment{};
		colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachment.blendEnable = VK_FALSE;
		colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
		colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
		colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
		colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
		colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
		colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional

		VkPipelineColorBlendStateCreateInfo colorBlending{};
		colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlending.logicOpEnable = VK_FALSE;
		colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
		colorBlending.attachmentCount = 1;
		colorBlending.pAttachments = &colorBlendAttachment;
		colorBlending.blendConstants[0] = 0.0f; // Optional
		colorBlending.blendConstants[1] = 0.0f; // Optional
		colorBlending.blendConstants[2] = 0.0f; // Optional
		colorBlending.blendConstants[3] = 0.0f; // Optional

		const auto& descriptorSetLayouts = basic3DShader.GetDescriptorSetLayouts();
		const auto& pushConstantRanges = basic3DShader.GetPushConstantRanges();

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
		pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
		pipelineLayoutInfo.pushConstantRangeCount = static_cast<uint32_t>(pushConstantRanges.size());
		pipelineLayoutInfo.pPushConstantRanges = pushConstantRanges.data();

		if (const VkResult result = vkCreatePipelineLayout(m_Device, &pipelineLayoutInfo, nullptr, &m_PipelineLayout); result != VK_SUCCESS)
		{
			KBR_CORE_ASSERT(false, "Failed to create pipeline layout! Result: {0}", VulkanHelpers::VkResultToString(result));
			throw std::runtime_error("failed to create pipeline layout!");
		}

		VkGraphicsPipelineCreateInfo pipelineInfo{};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount = 2;
		pipelineInfo.pStages = shaderStages;
		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &inputAssembly;
		pipelineInfo.pViewportState = &viewportState;
		pipelineInfo.pRasterizationState = &rasterizer;
		pipelineInfo.pMultisampleState = &multisampling;
		pipelineInfo.pDepthStencilState = nullptr; // Optional
		pipelineInfo.pColorBlendState = &colorBlending;
		pipelineInfo.pDynamicState = &dynamicState;

		pipelineInfo.layout = m_PipelineLayout;

		pipelineInfo.renderPass = m_RenderPass;
		pipelineInfo.subpass = 0;

		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
		pipelineInfo.basePipelineIndex = -1; // Optional

		if (const VkResult result = vkCreateGraphicsPipelines(m_Device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_GraphicsPipeline); result != VK_SUCCESS)
		{
			KBR_CORE_ASSERT(false, "Failed to create graphics pipeline! Result: {0}", VulkanHelpers::VkResultToString(result));
			throw std::runtime_error("failed to create graphics pipeline!");
		}
	}

	void VulkanContext::CreateVertexBuffer()
	{
		const float cubeVertices[] = {
			// Face 1: +X (Right)
			// Position           Normal             TexCoord
			// Triangle 1
			1.0f, -1.0f, -1.0f,   1.0f, 0.0f, 0.0f,   0.0f, 0.0f, // Bottom-left
			1.0f, -1.0f,  1.0f,   1.0f, 0.0f, 0.0f,   1.0f, 0.0f, // Bottom-right
			1.0f,  1.0f,  1.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f, // Top-right
			// Triangle 2
			1.0f, -1.0f, -1.0f,   1.0f, 0.0f, 0.0f,   0.0f, 0.0f, // Bottom-left
			1.0f,  1.0f,  1.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f, // Top-right
			1.0f,  1.0f, -1.0f,   1.0f, 0.0f, 0.0f,   0.0f, 1.0f, // Top-left

			// Face 2: -X (Left)
			// Triangle 1
			-1.0f, -1.0f,  1.0f,  -1.0f, 0.0f, 0.0f,   0.0f, 0.0f, // Bottom-left
			-1.0f, -1.0f, -1.0f,  -1.0f, 0.0f, 0.0f,   1.0f, 0.0f, // Bottom-right
			-1.0f,  1.0f, -1.0f,  -1.0f, 0.0f, 0.0f,   1.0f, 1.0f, // Top-right
			// Triangle 2
			-1.0f, -1.0f,  1.0f,  -1.0f, 0.0f, 0.0f,   0.0f, 0.0f, // Bottom-left
			-1.0f,  1.0f, -1.0f,  -1.0f, 0.0f, 0.0f,   1.0f, 1.0f, // Top-right
			-1.0f,  1.0f,  1.0f,  -1.0f, 0.0f, 0.0f,   0.0f, 1.0f, // Top-left

			// Face 3: +Y (Top)
			// Triangle 1
			-1.0f,  1.0f, -1.0f,   0.0f, 1.0f, 0.0f,   0.0f, 0.0f, // Bottom-left
			-1.0f,  1.0f,  1.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f, // Bottom-right
			 1.0f,  1.0f,  1.0f,   0.0f, 1.0f, 0.0f,   1.0f, 1.0f, // Top-right
			 // Triangle 2
			 -1.0f,  1.0f, -1.0f,   0.0f, 1.0f, 0.0f,   0.0f, 0.0f, // Bottom-left
			  1.0f,  1.0f,  1.0f,   0.0f, 1.0f, 0.0f,   1.0f, 1.0f, // Top-right
			  1.0f,  1.0f, -1.0f,   0.0f, 1.0f, 0.0f,   0.0f, 1.0f, // Top-left

			  // Face 4: -Y (Bottom)
			  // Triangle 1
			  -1.0f, -1.0f,  1.0f,   0.0f, -1.0f, 0.0f,   0.0f, 0.0f, // Bottom-left
			  -1.0f, -1.0f, -1.0f,   0.0f, -1.0f, 0.0f,   1.0f, 0.0f, // Bottom-right
			   1.0f, -1.0f, -1.0f,   0.0f, -1.0f, 0.0f,   1.0f, 1.0f, // Top-right
			   // Triangle 2
			   -1.0f, -1.0f,  1.0f,   0.0f, -1.0f, 0.0f,   0.0f, 0.0f, // Bottom-left
				1.0f, -1.0f, -1.0f,   0.0f, -1.0f, 0.0f,   1.0f, 1.0f, // Top-right
				1.0f, -1.0f,  1.0f,   0.0f, -1.0f, 0.0f,   0.0f, 1.0f, // Top-left

				// Face 5: +Z (Front)
				// Triangle 1
				-1.0f, -1.0f,  1.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f, // Bottom-left
				 1.0f, -1.0f,  1.0f,   0.0f, 0.0f, 1.0f,   1.0f, 0.0f, // Bottom-right
				 1.0f,  1.0f,  1.0f,   0.0f, 0.0f, 1.0f,   1.0f, 1.0f, // Top-right
				 // Triangle 2
				 -1.0f, -1.0f,  1.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f, // Bottom-left
				  1.0f,  1.0f,  1.0f,   0.0f, 0.0f, 1.0f,   1.0f, 1.0f, // Top-right
				 -1.0f,  1.0f,  1.0f,   0.0f, 0.0f, 1.0f,   0.0f, 1.0f, // Top-left

				 // Face 6: -Z (Back)
				 // Triangle 1
				  1.0f, -1.0f, -1.0f,   0.0f, 0.0f, -1.0f,   0.0f, 0.0f, // Bottom-left
				 -1.0f, -1.0f, -1.0f,   0.0f, 0.0f, -1.0f,   1.0f, 0.0f, // Bottom-right
				 -1.0f,  1.0f, -1.0f,   0.0f, 0.0f, -1.0f,   1.0f, 1.0f, // Top-right
				 // Triangle 2
				  1.0f, -1.0f, -1.0f,   0.0f, 0.0f, -1.0f,   0.0f, 0.0f, // Bottom-left
				 -1.0f,  1.0f, -1.0f,   0.0f, 0.0f, -1.0f,   1.0f, 1.0f, // Top-right
				  1.0f,  1.0f, -1.0f,   0.0f, 0.0f, -1.0f,   0.0f, 1.0f  // Top-left
		};
		constexpr uint32_t cubeVerticesSize = std::size(cubeVertices);

		m_VertexBuffer = CreateScope<VulkanVertexBuffer>(cubeVertices, cubeVerticesSize);
	}

	void VulkanContext::CreateIndexBuffer()
	{
				const uint32_t cubeIndices[] = {
			// Face 1: +X (Right)
			0, 1, 2, // Triangle 1
			0, 2, 3, // Triangle 2
			// Face 2: -X (Left)
			4, 5, 6, // Triangle 1
			4, 6, 7, // Triangle 2
			// Face 3: +Y (Top)
			8, 9, 10, // Triangle 1
			8, 10, 11, // Triangle 2
			// Face 4: -Y (Bottom)
			12, 13, 14, // Triangle 1
			12, 14, 15, // Triangle 2
			// Face 5: +Z (Front)
			16, 17, 18, // Triangle 1
			16, 18, 19, // Triangle 2
			// Face 6: -Z (Back)
			20, 21, 22, // Triangle 1
			20, 22, 23 // Triangle 2
		};
		constexpr uint32_t cubeIndicesSize = std::size(cubeIndices);
		m_IndexBuffer = CreateScope<VulkanIndexBuffer>(cubeIndices, cubeIndicesSize);
	}

	void VulkanContext::CreateFramebuffers()
	{
		m_SwapChainFramebuffers.resize(m_SwapChainImageViews.size());

		for (size_t i = 0; i < m_SwapChainImageViews.size(); ++i)
		{
			VkImageView attachments[] = {
				m_SwapChainImageViews[i]
			};

			VkFramebufferCreateInfo framebufferInfo{
				.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
				.pNext = nullptr,
				.flags = 0,
				.renderPass = m_RenderPass,
				.attachmentCount = 1,
				.pAttachments = attachments,
				.width = m_SwapChainExtent.width,
				.height = m_SwapChainExtent.height,
				.layers = 1,
			};

			if (const VkResult result = vkCreateFramebuffer(m_Device, &framebufferInfo, nullptr, &m_SwapChainFramebuffers[i]); result != VK_SUCCESS)
			{
				KBR_CORE_ASSERT(false, "Failed to create framebuffer! Result: {0}", VulkanHelpers::VkResultToString(result));
				throw std::runtime_error("failed to create framebuffer!");
			}
		}
	}

	void VulkanContext::CreateCommandPool()
	{
		const auto [graphicsFamily, presentFamily] = FindQueueFamilies(m_PhysicalDevice);

		if (!graphicsFamily.has_value())
		{
			throw std::runtime_error("failed to find graphics queue family!");
		}

		const VkCommandPoolCreateInfo poolInfo{
			.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
			.pNext = nullptr,
			.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT, // Optional
			.queueFamilyIndex = graphicsFamily.value(),
		};

		if (const VkResult result = vkCreateCommandPool(m_Device, &poolInfo, nullptr, &m_CommandPool); result != VK_SUCCESS)
		{
			KBR_CORE_ASSERT(false, "Failed to create command pool! Result: {0}", VulkanHelpers::VkResultToString(result));
			throw std::runtime_error("failed to create command pool!");
		}
	}

	void VulkanContext::CreateCommandBuffers()
	{
		m_CommandBuffers.resize(maxFramesInFlight);

		const VkCommandBufferAllocateInfo allocInfo{
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
			.pNext = nullptr,
			.commandPool = m_CommandPool,
			.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
			.commandBufferCount = static_cast<uint32_t>(m_CommandBuffers.size()),
		};

		if (const VkResult result = vkAllocateCommandBuffers(m_Device, &allocInfo, m_CommandBuffers.data()); result != VK_SUCCESS)
		{
			KBR_CORE_ASSERT(false, "Failed to allocate command buffers! Result: {0}", VulkanHelpers::VkResultToString(result));
			throw std::runtime_error("failed to allocate command buffers!");
		}
	}

	void VulkanContext::CreateSyncObjects()
	{
		m_ImageAvailableSemaphores.resize(maxFramesInFlight);
		m_RenderFinishedSemaphores.resize(maxFramesInFlight);
		m_InFlightFences.resize(maxFramesInFlight);


		VkSemaphoreCreateInfo semaphoreInfo{};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		/// Create the fence in a signaled state, since the first frame would wait
		/// infinitely for the fence to be reset before starting to render.
		VkFenceCreateInfo fenceInfo{};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		for (size_t i = 0; i < maxFramesInFlight; ++i)
		{
			if (vkCreateSemaphore(m_Device, &semaphoreInfo, nullptr, &m_ImageAvailableSemaphores[i]) != VK_SUCCESS ||
				vkCreateSemaphore(m_Device, &semaphoreInfo, nullptr, &m_RenderFinishedSemaphores[i]) != VK_SUCCESS ||
				vkCreateFence(m_Device, &fenceInfo, nullptr, &m_InFlightFences[i]) != VK_SUCCESS) {
				KBR_CORE_ASSERT(false, "Failed to create semaphores or fences!");
				throw std::runtime_error("failed to create semaphores or fences!");
			}
		}
	}

	VkCommandBuffer VulkanContext::GetOneTimeCommandBuffer() const 
	{
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = m_CommandPool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = 1;

		VkCommandBuffer commandBuffer;
		if (const VkResult result = vkAllocateCommandBuffers(m_Device, &allocInfo, &commandBuffer); result != VK_SUCCESS)
		{
			KBR_CORE_ASSERT(false, "Failed to allocate command buffer! Result: {0}", VulkanHelpers::VkResultToString(result));
			throw std::runtime_error("failed to allocate command buffer!");
		}

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		if (const VkResult result = vkBeginCommandBuffer(commandBuffer, &beginInfo); result != VK_SUCCESS)
		{
			KBR_CORE_ASSERT(false, "Failed to begin command buffer! Result: {0}", VulkanHelpers::VkResultToString(result));
			throw std::runtime_error("failed to begin command buffer!");
		}

		return commandBuffer;
	}

	void VulkanContext::SubmitCommandBuffer(const VkCommandBuffer commandBuffer) const 
	{
		constexpr uint64_t fenceWaitTimeout = 10000000000000;

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;

		if (const VkResult result = vkEndCommandBuffer(commandBuffer); result != VK_SUCCESS)
		{
			KBR_CORE_ASSERT(false, "Failed to end command buffer! Result: {0}", VulkanHelpers::VkResultToString(result));
			throw std::runtime_error("failed to end command buffer!");
		}

		VkFenceCreateInfo fenceInfo{};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = 0;

		VkFence fence;
		if (const VkResult result = vkCreateFence(m_Device, &fenceInfo, nullptr, &fence); result != VK_SUCCESS)
		{
			KBR_CORE_ASSERT(false, "Failed to create fence! Result: {0}", VulkanHelpers::VkResultToString(result));
			throw std::runtime_error("failed to create fence!");
		}

		if (const VkResult result = vkQueueSubmit(m_GraphicsQueue, 1, &submitInfo, fence); result != VK_SUCCESS)
		{
			KBR_CORE_ASSERT(false, "Failed to submit command buffer! Result: {0}", VulkanHelpers::VkResultToString(result));
			throw std::runtime_error("failed to submit command buffer!");
		}

		if (const VkResult result = vkWaitForFences(m_Device, 1, &fence, VK_TRUE, fenceWaitTimeout); result != VK_SUCCESS)
		{
			KBR_CORE_ASSERT(false, "Failed to wait for fence! Result: {0}", VulkanHelpers::VkResultToString(result));
			throw std::runtime_error("failed to wait for fence!");
		}

		vkDestroyFence(m_Device, fence, nullptr);
	}


	bool VulkanContext::CheckValidationLayerSupport()
	{
		uint32_t layerCount;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

		std::vector<VkLayerProperties> availableLayers(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

		for (const char* layerName : validationLayers)
		{
			bool layerFound = false;

			for (const auto& layerProperties : availableLayers)
			{
				if (strcmp(layerName, layerProperties.layerName) == 0)
				{
					layerFound = true;
					break;
				}
			}

			if (!layerFound)
			{
				return false;
			}
		}

		return true;
	}

	std::vector<const char*> VulkanContext::GetRequiredExtensions()
	{
		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

		if (enableValidationLayers)
		{
			extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		}

		return extensions;
	}

	void VulkanContext::PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
	{
		createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		createInfo.pfnUserCallback = DebugCallback;
	}

	bool VulkanContext::IsDeviceSuitable(const VkPhysicalDevice device) const
	{
		const QueueFamilyIndices indices = FindQueueFamilies(device);

		const bool extensionsSupported = CheckDeviceExtensionSupport(device);

		bool swapChainAdequate = false;
		if (extensionsSupported)
		{
			const SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(device);
			swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
		}

		return indices.IsComplete() && extensionsSupported && swapChainAdequate;
	}

	bool VulkanContext::CheckDeviceExtensionSupport(const VkPhysicalDevice device)
	{
		uint32_t extensionCount;
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

		std::vector<VkExtensionProperties> availableExtensions(extensionCount);
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

		std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

		for (const auto& extension : availableExtensions)
		{
			requiredExtensions.erase(extension.extensionName);
		}

		return requiredExtensions.empty();
	}

	QueueFamilyIndices VulkanContext::FindQueueFamilies() const
	{
		return FindQueueFamilies(m_PhysicalDevice);
	}

	QueueFamilyIndices VulkanContext::FindQueueFamilies(const VkPhysicalDevice device) const
	{
		QueueFamilyIndices indices;

		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

		int i = 0;
		for (const auto& queueFamily : queueFamilies)
		{
			if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
			{
				indices.graphicsFamily = i;
			}

			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(device, i, m_Surface, &presentSupport);

			if (presentSupport)
			{
				indices.presentFamily = i;
			}

			if (indices.IsComplete())
			{
				break;
			}

			i++;
		}

		return indices;
	}

	VulkanContext::SwapChainSupportDetails VulkanContext::QuerySwapChainSupport(const VkPhysicalDevice device) const
	{
		SwapChainSupportDetails details;

		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, m_Surface, &details.capabilities);

		uint32_t formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_Surface, &formatCount, nullptr);

		if (formatCount != 0)
		{
			details.formats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_Surface, &formatCount, details.formats.data());
		}

		uint32_t presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_Surface, &presentModeCount, nullptr);

		if (presentModeCount != 0)
		{
			details.presentModes.resize(presentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_Surface, &presentModeCount, details.presentModes.data());
		}

		return details;
	}

	VkSurfaceFormatKHR VulkanContext::ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
	{
		for (const auto& availableFormat : availableFormats)
		{
			if (availableFormat.format == VK_FORMAT_R8G8B8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
			{
				return availableFormat;
			}
		}

		return availableFormats[0];
	}

	VkPresentModeKHR VulkanContext::ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
	{
		for (const auto& availablePresentMode : availablePresentModes)
		{
			if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
			{
				return availablePresentMode;
			}
		}

		return VK_PRESENT_MODE_FIFO_KHR;
	}

	VkExtent2D VulkanContext::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) const
	{
		if (capabilities.currentExtent.width != UINT_MAX)
		{
			return capabilities.currentExtent;
		}

		/// If the current extent is not defined, we need to query the window size

		int width, height;
		glfwGetFramebufferSize(m_WindowHandle, &width, &height);

		VkExtent2D actualExtent = {
			static_cast<uint32_t>(width),
			static_cast<uint32_t>(height)
		};

		actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
		actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

		return actualExtent;
	}
}
