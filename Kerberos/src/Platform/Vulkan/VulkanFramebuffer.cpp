#include "kbrpch.h"
#include "VulkanFramebuffer.h"

#include <backends/imgui_impl_vulkan.h>

#include "VulkanContext.h"
#include "VulkanHelpers.h"

namespace Kerberos
{
	namespace Utils
	{
		static VkFormat FramebufferTextureFormatToVulkanFormat(const FramebufferTextureFormat format)
		{
			switch (format)
			{
			case FramebufferTextureFormat::RGBA8:           return VK_FORMAT_R8G8B8A8_UNORM;
			case FramebufferTextureFormat::DEPTH24STENCIL8: return VK_FORMAT_D24_UNORM_S8_UINT;
			case FramebufferTextureFormat::None:            return VK_FORMAT_UNDEFINED;
			}
			KBR_CORE_ASSERT(false, "Unknown FramebufferTextureFormat!");
			return VK_FORMAT_UNDEFINED;
		}

		static bool IsDepthFormat(const FramebufferTextureFormat format)
		{
			return format == FramebufferTextureFormat::DEPTH24STENCIL8;
		}
	}

	VulkanFramebuffer::VulkanFramebuffer(FramebufferSpecification spec)
		: m_Specification(std::move(spec))
	{
		KBR_PROFILE_FUNCTION();

		for (auto format : m_Specification.Attachments.Attachments)
		{
			if (Utils::IsDepthFormat(format.TextureFormat))
			{
				m_DepthAttachmentSpec = format;
			}
			else
			{
				m_ColorAttachmentSpecs.emplace_back(format);
			}
		}

		KBR_CORE_ASSERT(!m_ColorAttachmentSpecs.empty(), "Framebuffer must have at least one color attachment!");
		KBR_CORE_ASSERT(m_DepthAttachmentSpec.TextureFormat != FramebufferTextureFormat::None, "Framebuffer must have a depth attachment!");
		KBR_CORE_ASSERT(m_Specification.Width > 0 && m_Specification.Height > 0, "Framebuffer dimensions must be greater than zero!");
		KBR_CORE_ASSERT(m_Specification.Samples > 0, "Framebuffer samples must be greater than zero!");

		KBR_CORE_ASSERT(m_Specification.Samples == 1, "Vulkan currently doesn't support multisampling!");
		KBR_CORE_ASSERT(m_ColorAttachmentSpecs.size() == 1, "VulkanFramebuffer currently only supports one color attachment!");

		Invalidate();
	}

	VulkanFramebuffer::~VulkanFramebuffer() 
	{
		KBR_PROFILE_FUNCTION();

		ReleaseResources();
	}

	void VulkanFramebuffer::Invalidate()
	{
		KBR_PROFILE_FUNCTION();

		const VulkanContext& context = VulkanContext::Get();

		vkDeviceWaitIdle(context.GetDevice());

		if (m_Framebuffer != VK_NULL_HANDLE)
		{
			ReleaseResources();
		}

		{
			size_t colorAttachmentCount = m_ColorAttachmentSpecs.size();
			m_ColorAttachmentMemories.resize(colorAttachmentCount);
			m_ColorAttachments.resize(colorAttachmentCount);
			m_ColorAttachmentViews.resize(colorAttachmentCount);
		}


		const VkFormat colorFormat = Utils::FramebufferTextureFormatToVulkanFormat(m_ColorAttachmentSpecs[0].TextureFormat);

		CreateImage(
			m_Specification.Width, 
			m_Specification.Height, 
			colorFormat, 
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			m_ColorAttachments[0], m_ColorAttachmentMemories[0]);

		CreateImageView(m_ColorAttachments[0], colorFormat, VK_IMAGE_ASPECT_COLOR_BIT, m_ColorAttachmentViews[0]);

		std::vector<VkAttachmentDescription> attachmentDescriptions;
		attachmentDescriptions.resize(m_ColorAttachmentSpecs.size());
		for (size_t i = 0; i < m_ColorAttachmentSpecs.size(); ++i)
		{
			attachmentDescriptions[i].format = Utils::FramebufferTextureFormatToVulkanFormat(m_ColorAttachmentSpecs[i].TextureFormat);
			attachmentDescriptions[i].samples = VK_SAMPLE_COUNT_1_BIT; /// Make this customizable when supporting multisampling
			attachmentDescriptions[i].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			attachmentDescriptions[i].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			attachmentDescriptions[i].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			attachmentDescriptions[i].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			attachmentDescriptions[i].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			attachmentDescriptions[i].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		}


		if (m_DepthAttachmentSpec.TextureFormat != FramebufferTextureFormat::None)
		{
			const VkFormat depthFormat = Utils::FramebufferTextureFormatToVulkanFormat(m_DepthAttachmentSpec.TextureFormat);

			CreateImage(
				m_Specification.Width, 
				m_Specification.Height, 
				depthFormat, 
				VK_IMAGE_TILING_OPTIMAL,
				VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
				m_DepthAttachment, m_DepthAttachmentMemory);

			CreateImageView(m_DepthAttachment, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT, m_DepthAttachmentView);

			VkAttachmentDescription depthAttachmentDescription{};
			depthAttachmentDescription.format = depthFormat;
			depthAttachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT; /// Make this customizable when supporting multisampling
			depthAttachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			depthAttachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			depthAttachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			depthAttachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			depthAttachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			depthAttachmentDescription.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

			attachmentDescriptions.push_back(depthAttachmentDescription);
		}

		/// Subpass attachments
		VkAttachmentReference colorAttachmentRef{};
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentReference depthAttachmentRef{};
		if (m_DepthAttachmentSpec.TextureFormat != FramebufferTextureFormat::None)
		{
			depthAttachmentRef.attachment = static_cast<uint32_t>(attachmentDescriptions.size() - 1);
			depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		}

		/// Subpass creation
		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;
		if (m_DepthAttachmentSpec.TextureFormat != FramebufferTextureFormat::None)
		{
			subpass.pDepthStencilAttachment = &depthAttachmentRef;
		}
		else
		{
			subpass.pDepthStencilAttachment = nullptr;
		}

		std::vector<VkSubpassDependency> dependencies(2);

		/// Dependency for transitioning color attachment into the rendes pass
		dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
		dependencies[0].dstSubpass = 0;
		/// Wait for previous operations to finish before starting this subpass
		dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		/// Transition before starting the subpass
		dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependencies[0].srcAccessMask = 0;
		dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

		/// Dependency for transitioning color attachment after the render pass
		dependencies[1].srcSubpass = 0;
		dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
		dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT; /// Available for sampling in fragment shader
		dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		dependencies[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

		/// Create render pass
		VkRenderPassCreateInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = static_cast<uint32_t>(attachmentDescriptions.size());
		renderPassInfo.pAttachments = attachmentDescriptions.data();
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;
		renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
		renderPassInfo.pDependencies = dependencies.data();

		if (vkCreateRenderPass(context.GetDevice(), &renderPassInfo, nullptr, &m_RenderPass) != VK_SUCCESS)
		{
			KBR_CORE_ASSERT(false, "Failed to create render pass!");
			return;
		}

		/// Create framebuffer
		std::vector<VkImageView> framebufferAttachments = m_ColorAttachmentViews;
		if (m_DepthAttachmentSpec.TextureFormat != FramebufferTextureFormat::None)
		{
			framebufferAttachments.push_back(m_DepthAttachmentView);
		}

		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = m_RenderPass;
		framebufferInfo.attachmentCount = static_cast<uint32_t>(framebufferAttachments.size());
		framebufferInfo.pAttachments = framebufferAttachments.data();
		framebufferInfo.width = m_Specification.Width;
		framebufferInfo.height = m_Specification.Height;
		framebufferInfo.layers = 1;

		if (vkCreateFramebuffer(context.GetDevice(), &framebufferInfo, nullptr, &m_Framebuffer) != VK_SUCCESS)
		{
			KBR_CORE_ASSERT(false, "Failed to create framebuffer!");
			return;
		}

		/// Create sampler for color attachment
		VkSamplerCreateInfo samplerInfo{};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = VK_FILTER_LINEAR;
		samplerInfo.minFilter = VK_FILTER_LINEAR;
		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR; /// Make this customizable when supporting mipmapping
		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerInfo.anisotropyEnable = VK_FALSE; /// Set to VK_TRUE if anisotropic filtering is supported
		samplerInfo.minLod = 0.0f;
		samplerInfo.maxLod = 0.0f;
		samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
		samplerInfo.unnormalizedCoordinates = VK_FALSE;

		if (vkCreateSampler(context.GetDevice(), &samplerInfo, nullptr, &m_ColorAttachmentSampler) != VK_SUCCESS)
		{
			KBR_CORE_ASSERT(false, "Failed to create color attachment sampler!");
			return;
		}

		/// Create command pool and command buffer for framebuffer operations
		VkCommandPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		poolInfo.queueFamilyIndex = context.GetGraphicsQueueFamilyIndex();

		if (vkCreateCommandPool(context.GetDevice(), &poolInfo, nullptr, &m_CommandPool) != VK_SUCCESS)
		{
			KBR_CORE_ASSERT(false, "Failed to create command pool!");
			return;
		}

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = m_CommandPool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = 1;

		if (vkAllocateCommandBuffers(context.GetDevice(), &allocInfo, &m_CommandBuffer) != VK_SUCCESS)
		{
			KBR_CORE_ASSERT(false, "Failed to allocate command buffer!");
			return;
		}

		/// Create fence
		VkFenceCreateInfo fenceInfo{};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
		if (vkCreateFence(context.GetDevice(), &fenceInfo, nullptr, &m_Fence) != VK_SUCCESS)
		{
			KBR_CORE_ASSERT(false, "Failed to create fence!");
			return;
		}

		/// Create descriptor set for color attachment
		m_ColorAttachmentDescriptorSet = ImGui_ImplVulkan_AddTexture(m_ColorAttachmentSampler, m_ColorAttachmentViews[0], VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	}

	void VulkanFramebuffer::Bind()
	{
		if (const VkResult result = vkWaitForFences(VulkanContext::Get().GetDevice(), 1, &m_Fence, VK_TRUE, UINT64_MAX); result != VK_SUCCESS)
		{
			KBR_CORE_ASSERT(false, "Failed to wait for fence!");
			return;
		}

		if (const VkResult result = vkResetFences(VulkanContext::Get().GetDevice(), 1, &m_Fence); result != VK_SUCCESS)
		{
			KBR_CORE_ASSERT(false, "Failed to reset fence!");
			return;
		}

		vkResetCommandBuffer(m_CommandBuffer, 0);

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		
		if (vkBeginCommandBuffer(m_CommandBuffer, &beginInfo) != VK_SUCCESS)
		{
			KBR_CORE_ASSERT(false, "Failed to begin command buffer!");
			return;
		}

		std::vector<VkClearValue> clearValues;
		clearValues.push_back({{{0.0f, 0.0f, 0.0f, 1.0f}}});
		if (m_DepthAttachmentSpec.TextureFormat != FramebufferTextureFormat::None)
		{
			clearValues.push_back({{{1.0f, 0}}});
		}

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = m_RenderPass;
		renderPassInfo.framebuffer = m_Framebuffer;
		renderPassInfo.renderArea.offset = { .x = 0, .y = 0 };
		renderPassInfo.renderArea.extent.width = m_Specification.Width;
		renderPassInfo.renderArea.extent.height = m_Specification.Height;
		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassInfo.pClearValues = clearValues.data();

		vkCmdBeginRenderPass(m_CommandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = static_cast<float>(m_Specification.Height);
		viewport.width = static_cast<float>(m_Specification.Width);
		viewport.height = -static_cast<float>(m_Specification.Height); // Invert Y-axis for Vulkan
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(m_CommandBuffer, 0, 1, &viewport);

		VkRect2D scissor{};
		scissor.offset = { .x = 0, .y = 0 };
		scissor.extent.width = m_Specification.Width;
		scissor.extent.height = m_Specification.Height;
		vkCmdSetScissor(m_CommandBuffer, 0, 1, &scissor);
	}

	void VulkanFramebuffer::Unbind()
	{
		vkCmdEndRenderPass(m_CommandBuffer);

		if (const VkResult result = vkEndCommandBuffer(m_CommandBuffer); result != VK_SUCCESS)
		{
			KBR_CORE_ASSERT(false, "Failed to end command buffer!");
			return;
		}

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &m_CommandBuffer;
		if (vkQueueSubmit(VulkanContext::Get().GetGraphicsQueue(), 1, &submitInfo, m_Fence) != VK_SUCCESS)
		{
			KBR_CORE_ASSERT(false, "Failed to submit command buffer!");
			return;
		}
	}

	void VulkanFramebuffer::Resize(uint32_t width, uint32_t height)
	{
		KBR_PROFILE_FUNCTION();

		if (width == 0 || height == 0)
		{
			KBR_CORE_WARN("Attempted to resize framebuffer to {0}, {1}", width, height);
			return;
		}

		m_Specification.Width = width;
		m_Specification.Height = height;

		Invalidate();
	}

	int VulkanFramebuffer::ReadPixel(uint32_t attachmentIndex, int x, int y) 
	{
		// TODO
		return -1;
	}

	void VulkanFramebuffer::ClearAttachment(uint32_t attachmentIndex, int value) 
	{
	}

	uint64_t VulkanFramebuffer::GetColorAttachmentRendererID(uint32_t index) const
	{
		return reinterpret_cast<ImTextureID>(m_ColorAttachmentDescriptorSet);
	}

	void VulkanFramebuffer::ReleaseResources()
	{
		const VulkanContext& context = VulkanContext::Get();
		const VkDevice device = context.GetDevice();

		vkDeviceWaitIdle(device);

		vkDestroySampler(device, m_ColorAttachmentSampler, nullptr);
		m_ColorAttachmentSampler = VK_NULL_HANDLE;

		vkDestroyFence(device, m_Fence, nullptr);
		m_Fence = VK_NULL_HANDLE;

		vkFreeCommandBuffers(device, m_CommandPool, 1, &m_CommandBuffer);
		vkDestroyCommandPool(device, m_CommandPool, nullptr);
		m_CommandPool = VK_NULL_HANDLE;
		m_CommandBuffer = VK_NULL_HANDLE;

		vkDestroyFramebuffer(device, m_Framebuffer, nullptr);
		vkDestroyRenderPass(device, m_RenderPass, nullptr);
		m_Framebuffer = VK_NULL_HANDLE;
		m_RenderPass = VK_NULL_HANDLE;

		vkDestroyImageView(device, m_DepthAttachmentView, nullptr);
		vkDestroyImage(device, m_DepthAttachment, nullptr);
		vkFreeMemory(device, m_DepthAttachmentMemory, nullptr);
		m_DepthAttachment = VK_NULL_HANDLE;
		m_DepthAttachmentMemory = VK_NULL_HANDLE;
		m_DepthAttachmentView = VK_NULL_HANDLE;

		for (size_t i = 0; i < m_ColorAttachments.size(); ++i)
		{
			vkDestroyImageView(device, m_ColorAttachmentViews[i], nullptr);
			vkDestroyImage(device, m_ColorAttachments[i], nullptr);
			vkFreeMemory(device, m_ColorAttachmentMemories[i], nullptr);
			m_ColorAttachments[i] = VK_NULL_HANDLE;
			m_ColorAttachmentMemories[i] = VK_NULL_HANDLE;
			m_ColorAttachmentViews[i] = VK_NULL_HANDLE;
		}
		m_ColorAttachments.clear();
		m_ColorAttachmentMemories.clear();
		m_ColorAttachmentViews.clear();

		ImGui_ImplVulkan_RemoveTexture(m_ColorAttachmentDescriptorSet);
	}

	void VulkanFramebuffer::CreateImage(const uint32_t width, const uint32_t height, const VkFormat format, const VkImageTiling tiling,
		const VkImageUsageFlags usage, const VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& memory) 
	{
		const VulkanContext& context = VulkanContext::Get();

		VkImageCreateInfo imageInfo{};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.extent.width = width;
		imageInfo.extent.height = height;
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = 1;
		imageInfo.arrayLayers = 1;
		imageInfo.format = format;
		imageInfo.tiling = tiling;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageInfo.usage = usage;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT; /// Make this also customizable when supporting multisampling
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		if (vkCreateImage(context.GetDevice(), &imageInfo, nullptr, &image) != VK_SUCCESS)
		{
			KBR_CORE_ASSERT(false, "Failed to create image!");
			return;
		}

		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements(context.GetDevice(), image, &memRequirements);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = VulkanHelpers::FindMemoryType(context.GetPhysicalDevice(), memRequirements.memoryTypeBits, properties);

		if (vkAllocateMemory(context.GetDevice(), &allocInfo, nullptr, &memory) != VK_SUCCESS)
		{
			KBR_CORE_ASSERT(false, "Failed to allocate image memory!");
			return;
		}

		if (vkBindImageMemory(context.GetDevice(), image, memory, 0) != VK_SUCCESS)
		{
			KBR_CORE_ASSERT(false, "Failed to bind image memory!");
		}
	}

	void VulkanFramebuffer::CreateImageView(const VkImage image, const VkFormat format, const VkImageAspectFlags flags,
		VkImageView& imageView) 
	{
		VkImageViewCreateInfo viewInfo{};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = image;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = format;
		viewInfo.subresourceRange.aspectMask = flags;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;

		if (vkCreateImageView(VulkanContext::Get().GetDevice(), &viewInfo, nullptr, &imageView) != VK_SUCCESS)
		{
			KBR_CORE_ASSERT(false, "Failed to create image view!");
		}
	}
}
