#include "kbrpch.h"
#include "VulkanTexture.h"

#include <stb_image.h>
#include <backends/imgui_impl_vulkan.h>

#include "VulkanContext.h"

namespace Kerberos
{
	VulkanTexture2D::VulkanTexture2D(const std::string& path)
		: m_Path(path)
	{
		KBR_PROFILE_FUNCTION();

		int width, height, channels;

		//stbi_set_flip_vertically_on_load(true);

		stbi_uc* imageData = nullptr;
		{
			KBR_PROFILE_SCOPE("stbi_load - VulkanTexture2D::VulkanTexture2D(const std::string& path)");
			/// Set the desired number of channels to 4 (RGBA) for Vulkan compatibility
			imageData = stbi_load(path.c_str(), &width, &height, &channels, 4);
		}

		KBR_ASSERT(imageData, "Failed to load image!");

		m_Width = static_cast<unsigned int>(width);
		m_Height = static_cast<unsigned int>(height);

		uint32_t imageSize = m_Width * m_Height * 4;

		const VulkanContext& context = VulkanContext::Get();
		const VkDevice device = context.GetDevice();
		const VkPhysicalDevice physicalDevice = context.GetPhysicalDevice();

        VkResult err;

        /// Create the image.
        {
            VkImageCreateInfo imageInfo = {};
            imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
            imageInfo.imageType = VK_IMAGE_TYPE_2D;
            imageInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
            imageInfo.extent.width = m_Width;
            imageInfo.extent.height = m_Height;
            imageInfo.extent.depth = 1;
            imageInfo.mipLevels = 1;
            imageInfo.arrayLayers = 1;
            imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
            imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
            imageInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
            imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
            imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            err = vkCreateImage(device, &imageInfo, nullptr, &m_Image);

            VkMemoryRequirements req;
            vkGetImageMemoryRequirements(device, m_Image, &req);
            VkMemoryAllocateInfo allocInfo = {};

            allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            allocInfo.allocationSize = req.size;
            allocInfo.memoryTypeIndex = VulkanHelpers::FindMemoryType(physicalDevice, req.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

            err = vkAllocateMemory(device, &allocInfo, nullptr, &m_ImageMemory);

            err = vkBindImageMemory(device, m_Image, m_ImageMemory, 0);
        }

        /// Create the Image View
        {
            VkImageViewCreateInfo imageViewInfo = {};
            imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            imageViewInfo.image = m_Image;
            imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            imageViewInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
            imageViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            imageViewInfo.subresourceRange.levelCount = 1;
            imageViewInfo.subresourceRange.layerCount = 1;

            err = vkCreateImageView(device, &imageViewInfo, nullptr, &m_ImageView);
        }

        /// Create Sampler
        {
            VkSamplerCreateInfo samplerInfo{};
            samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
            samplerInfo.magFilter = VK_FILTER_LINEAR;
            samplerInfo.minFilter = VK_FILTER_LINEAR;
            samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
            samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT; // outside image bounds just use border color
            samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
            samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
            samplerInfo.minLod = -1000;
            samplerInfo.maxLod = 1000;
            samplerInfo.maxAnisotropy = 1.0f;

            err = vkCreateSampler(device, &samplerInfo, nullptr, &m_Sampler);
        }

        /// Create Descriptor Set using ImGUI's implementation
        m_DescriptorSet = ImGui_ImplVulkan_AddTexture(m_Sampler, m_ImageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

        /// Create Upload Buffer
        {
            VkBufferCreateInfo bufferInfo = {};
            bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
            bufferInfo.size = imageSize;
            bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
            bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

            err = vkCreateBuffer(device, &bufferInfo, nullptr, &m_UploadBuffer);

            VkMemoryRequirements req;
            vkGetBufferMemoryRequirements(device, m_UploadBuffer, &req);

            VkMemoryAllocateInfo allocInfo = {};
            allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            allocInfo.allocationSize = req.size;
            allocInfo.memoryTypeIndex = VulkanHelpers::FindMemoryType(physicalDevice, req.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

            err = vkAllocateMemory(device, &allocInfo, nullptr, &m_UploadBufferMemory);

            err = vkBindBufferMemory(device, m_UploadBuffer, m_UploadBufferMemory, 0);
        }

        /// Upload to Buffer:
        {
            void* map = nullptr;

            err = vkMapMemory(device, m_UploadBufferMemory, 0, imageSize, 0, &map);

            memcpy(map, imageData, imageSize);

            VkMappedMemoryRange range[1] = {};
            range[0].sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
            range[0].memory = m_UploadBufferMemory;
            range[0].size = imageSize;

            err = vkFlushMappedMemoryRanges(device, 1, range);

            vkUnmapMemory(device, m_UploadBufferMemory);
        }

        stbi_image_free(imageData);

		VkCommandPool commandPool = context.GetCommandPool();
		VkCommandBuffer commandBuffer = context.GetCommandBuffer();
        /*{
            VkCommandBufferAllocateInfo allocInfo{};
            allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            allocInfo.commandPool = commandPool;
            allocInfo.commandBufferCount = 1;

            err = vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

            VkCommandBufferBeginInfo beginInfo = {};
            beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            beginInfo.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

            err = vkBeginCommandBuffer(commandBuffer, &beginInfo);
        }*/

        /// Copy to Image
        {
            VkImageMemoryBarrier copyBarrier[1] = {};
            copyBarrier[0].sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            copyBarrier[0].dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            copyBarrier[0].oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            copyBarrier[0].newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            copyBarrier[0].srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            copyBarrier[0].dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            copyBarrier[0].image = m_Image;
            copyBarrier[0].subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            copyBarrier[0].subresourceRange.levelCount = 1;
            copyBarrier[0].subresourceRange.layerCount = 1;

            vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, copyBarrier);

            VkBufferImageCopy region = {};
            region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            region.imageSubresource.layerCount = 1;
            region.imageExtent.width = m_Width;
            region.imageExtent.height = m_Height;
            region.imageExtent.depth = 1;
            vkCmdCopyBufferToImage(commandBuffer, m_UploadBuffer, m_Image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

            VkImageMemoryBarrier useBarrier[1] = {};
            useBarrier[0].sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            useBarrier[0].srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            useBarrier[0].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            useBarrier[0].oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            useBarrier[0].newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            useBarrier[0].srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            useBarrier[0].dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            useBarrier[0].image = m_Image;
            useBarrier[0].subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            useBarrier[0].subresourceRange.levelCount = 1;
            useBarrier[0].subresourceRange.layerCount = 1;

            vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, useBarrier);
        }

        /// End command buffer
		context.SubmitCommandBuffer(commandBuffer);
        /*{
            VkSubmitInfo endInfo = {};
            endInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            endInfo.commandBufferCount = 1;
            endInfo.pCommandBuffers = &commandBuffer;

            err = vkEndCommandBuffer(commandBuffer);

            err = vkQueueSubmit(g_Queue, 1, &endInfo, VK_NULL_HANDLE);

            err = vkDeviceWaitIdle(device);
        }*/

        m_RendererID = reinterpret_cast<ImTextureID>(m_DescriptorSet);
	}

	VulkanTexture2D::VulkanTexture2D(uint32_t width, uint32_t height)
	{
		
	}

	VulkanTexture2D::~VulkanTexture2D()
	{
		CleanupResources();
	}

	void VulkanTexture2D::Bind(uint32_t slot) const
	{
		
	}

	void VulkanTexture2D::SetData(void* data, uint32_t size)
	{
		
	}

	void VulkanTexture2D::CleanupResources() const 
    {
		KBR_PROFILE_FUNCTION();

		const VulkanContext& context = VulkanContext::Get();
		const VkDevice gDevice = context.GetDevice();

        vkFreeMemory(gDevice, m_UploadBufferMemory, nullptr);
        vkDestroyBuffer(gDevice, m_UploadBuffer, nullptr);
        vkDestroySampler(gDevice, m_Sampler, nullptr);
        vkDestroyImageView(gDevice, m_ImageView, nullptr);
        vkDestroyImage(gDevice, m_Image, nullptr);
        vkFreeMemory(gDevice, m_ImageMemory, nullptr);
        ImGui_ImplVulkan_RemoveTexture(m_DescriptorSet);
	}
}
