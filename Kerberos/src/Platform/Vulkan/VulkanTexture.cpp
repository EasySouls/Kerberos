#include "kbrpch.h"
#include "VulkanTexture.h"

#include <stb_image.h>
#include <backends/imgui_impl_vulkan.h>

#include "VulkanContext.h"

namespace Kerberos
{
    namespace Utils
    {
		/// This is really not optimised and should be replaced with a better solution.
        void TransitionImageLayout(const VkCommandBuffer commandBuffer, const VkImage image, VkFormat format, const VkImageLayout oldLayout, const VkImageLayout newLayout)
        {
            VkImageMemoryBarrier barrier{};
            barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            barrier.oldLayout = oldLayout;
            barrier.newLayout = newLayout;
            barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.image = image;
            barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            barrier.subresourceRange.baseMipLevel = 0;
            barrier.subresourceRange.levelCount = 1;
            barrier.subresourceRange.baseArrayLayer = 0;
            barrier.subresourceRange.layerCount = 1;

            VkPipelineStageFlags sourceStage;
            VkPipelineStageFlags destinationStage;

            if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
            {
                barrier.srcAccessMask = 0;
                barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
                destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            }
            else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
            {
                barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
                sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
                destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            }
            else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
            {
                // For creating an empty texture and directly making it shader readable
                barrier.srcAccessMask = 0;
                barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT; // Or 0 if no read access needed immediately after transition
                sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
                destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT; // Or PIPELINE_STAGE_ALL_COMMANDS_BIT if unsure
            }
            else if (oldLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
            {
                barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
                barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                sourceStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
                destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            }
            else
            {
                KBR_CORE_ERROR("Unsupported layout transition!");
                return;
            }

            vkCmdPipelineBarrier(
                commandBuffer,
                sourceStage, destinationStage,
                0,
                0, nullptr,
                0, nullptr,
                1, &barrier
            );
        }
    }

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

		m_Spec.Width = static_cast<unsigned int>(width);
		m_Spec.Height = static_cast<unsigned int>(height);

		uint32_t imageSize = m_Spec.Width * m_Spec.Height * 4;

		const VulkanContext& context = VulkanContext::Get();
		const VkDevice device = context.GetDevice();
		const VkPhysicalDevice physicalDevice = context.GetPhysicalDevice();

		VkResult err = VK_SUCCESS;

        /// Create the image.
        {
            VkImageCreateInfo imageInfo = {};
            imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
            imageInfo.imageType = VK_IMAGE_TYPE_2D;
            imageInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
            imageInfo.extent.width = m_Spec.Width;
            imageInfo.extent.height = m_Spec.Height;
            imageInfo.extent.depth = 1;
            imageInfo.mipLevels = 1;
            imageInfo.arrayLayers = 1;
            imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
            imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
            imageInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
            imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
            imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

            if (err = vkCreateImage(device, &imageInfo, nullptr, &m_Image); err != VK_SUCCESS)
            {
                KBR_ERROR("Failed to create Vulkan image: {}", VulkanHelpers::VkResultToString(err));
                return;
			}

            VkMemoryRequirements req;
            vkGetImageMemoryRequirements(device, m_Image, &req);
            VkMemoryAllocateInfo allocInfo = {};

            allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            allocInfo.allocationSize = req.size;
            allocInfo.memoryTypeIndex = VulkanHelpers::FindMemoryType(physicalDevice, req.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

            err = vkAllocateMemory(device, &allocInfo, nullptr, &m_ImageMemory);
            if (err != VK_SUCCESS)
            {
                KBR_ERROR("Failed to allocate Vulkan image memory: {}", VulkanHelpers::VkResultToString(err));
                return;
			}

            err = vkBindImageMemory(device, m_Image, m_ImageMemory, 0);
            if (err != VK_SUCCESS)
            {
                KBR_ERROR("Failed to bind Vulkan image memory: {}", VulkanHelpers::VkResultToString(err));
                return;
            }
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
            if (err != VK_SUCCESS)
            {
                KBR_ERROR("Failed to create Vulkan image view: {}", VulkanHelpers::VkResultToString(err));
                return;
			}
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
            if (err != VK_SUCCESS)
            {
                KBR_ERROR("Failed to create Vulkan sampler: {}", VulkanHelpers::VkResultToString(err));
                return;
            }
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
            if (err != VK_SUCCESS)
            {
                KBR_ERROR("Failed to create Vulkan upload buffer: {}", VulkanHelpers::VkResultToString(err));
                return;
			}

            VkMemoryRequirements req;
            vkGetBufferMemoryRequirements(device, m_UploadBuffer, &req);

            VkMemoryAllocateInfo allocInfo = {};
            allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            allocInfo.allocationSize = req.size;
            allocInfo.memoryTypeIndex = VulkanHelpers::FindMemoryType(physicalDevice, req.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

            err = vkAllocateMemory(device, &allocInfo, nullptr, &m_UploadBufferMemory);
            if (err != VK_SUCCESS)
            {
                KBR_ERROR("Failed to allocate Vulkan upload buffer memory: {}", VulkanHelpers::VkResultToString(err));
                return;
            }

            err = vkBindBufferMemory(device, m_UploadBuffer, m_UploadBufferMemory, 0);
            if (err != VK_SUCCESS)
            {
                KBR_ERROR("Failed to bind Vulkan upload buffer memory: {}", VulkanHelpers::VkResultToString(err));
                return;
			}
        }

        /// Upload to Buffer:
        {
            void* map = nullptr;

            err = vkMapMemory(device, m_UploadBufferMemory, 0, imageSize, 0, &map);
            if (err != VK_SUCCESS)
            {
                KBR_ERROR("Failed to map Vulkan upload buffer memory: {}", VulkanHelpers::VkResultToString(err));
                return;
            }

            memcpy(map, imageData, imageSize);

            VkMappedMemoryRange range[1] = {};
            range[0].sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
            range[0].memory = m_UploadBufferMemory;
            range[0].size = imageSize;

            err = vkFlushMappedMemoryRanges(device, 1, range);
            if (err != VK_SUCCESS)
            {
                KBR_ERROR("Failed to flush Vulkan upload buffer memory: {}", VulkanHelpers::VkResultToString(err));
                return;
			}

            vkUnmapMemory(device, m_UploadBufferMemory);
        }

        stbi_image_free(imageData);

		VkCommandBuffer commandBuffer = context.GetOneTimeCommandBuffer();

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
            region.imageExtent.width = m_Spec.Width;
            region.imageExtent.height = m_Spec.Height;
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
	}

	VulkanTexture2D::VulkanTexture2D(const TextureSpecification& spec, Buffer data)
		: m_Spec(spec)
	{
        KBR_PROFILE_FUNCTION();

        KBR_ASSERT(spec.Width > 0 && spec.Height > 0, "Texture dimensions must be greater than 0");

        uint32_t imageSize = spec.Width * spec.Height * 4; // 4 bytes per pixel (RGBA)

        const VulkanContext& context = VulkanContext::Get();
        const VkDevice device = context.GetDevice();
        const VkPhysicalDevice physicalDevice = context.GetPhysicalDevice();
        VkResult err;

        /// Create the image
        {
            VkImageCreateInfo imageInfo{};
            imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
            imageInfo.imageType = VK_IMAGE_TYPE_2D;
            imageInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
            imageInfo.extent.width = spec.Width;
            imageInfo.extent.height = spec.Height;
            imageInfo.extent.depth = 1;
            imageInfo.mipLevels = 1;
            imageInfo.arrayLayers = 1;
            imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
            imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
            // Need TRANSFER_DST_BIT for SetData, SAMPLED_BIT for sampling
            imageInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
            imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
            imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; // Will transition it

            err = vkCreateImage(device, &imageInfo, nullptr, &m_Image);
            if (err != VK_SUCCESS)
            {
                KBR_ERROR("Failed to create Vulkan image (empty): {}", VulkanHelpers::VkResultToString(err));
                return;
            }

            VkMemoryRequirements req;
            vkGetImageMemoryRequirements(device, m_Image, &req);
            VkMemoryAllocateInfo allocInfo{};
            allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            allocInfo.allocationSize = req.size;
            allocInfo.memoryTypeIndex = VulkanHelpers::FindMemoryType(physicalDevice, req.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

            err = vkAllocateMemory(device, &allocInfo, nullptr, &m_ImageMemory);
            if (err != VK_SUCCESS)
            {
                KBR_ERROR("Failed to allocate Vulkan image memory (empty): {}", VulkanHelpers::VkResultToString(err));
                vkDestroyImage(device, m_Image, nullptr);
                return;
            }
            err = vkBindImageMemory(device, m_Image, m_ImageMemory, 0);
            if (err != VK_SUCCESS)
            {
                KBR_ERROR("Failed to bind Vulkan image memory (empty): {}", VulkanHelpers::VkResultToString(err));
                vkFreeMemory(device, m_ImageMemory, nullptr);
                vkDestroyImage(device, m_Image, nullptr);
                return;
            }
        }

        /// Create the Image View
        {
            VkImageViewCreateInfo imageViewInfo{};
            imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            imageViewInfo.image = m_Image;
            imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            imageViewInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
            imageViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            imageViewInfo.subresourceRange.levelCount = 1;
            imageViewInfo.subresourceRange.layerCount = 1;
            err = vkCreateImageView(device, &imageViewInfo, nullptr, &m_ImageView);
            if (err != VK_SUCCESS)
            {
                KBR_ERROR("Failed to create Vulkan image view (empty): {}", VulkanHelpers::VkResultToString(err));
                vkFreeMemory(device, m_ImageMemory, nullptr);
                vkDestroyImage(device, m_Image, nullptr);
                return;
            }
        }

        /// Create Sampler
        {
            VkSamplerCreateInfo samplerInfo{};
            samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
            samplerInfo.magFilter = VK_FILTER_LINEAR;
            samplerInfo.minFilter = VK_FILTER_LINEAR;
            samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
            samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
            samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
            samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
            samplerInfo.minLod = 0.0f;
            samplerInfo.maxLod = 1.0f; // Or VK_LOD_CLAMP_NONE if mipLevels > 1
            samplerInfo.maxAnisotropy = 1.0f;
            err = vkCreateSampler(device, &samplerInfo, nullptr, &m_Sampler);
            if (err != VK_SUCCESS)
            {
                KBR_ERROR("Failed to create Vulkan sampler (empty): {}", VulkanHelpers::VkResultToString(err));
                vkDestroyImageView(device, m_ImageView, nullptr);
                vkFreeMemory(device, m_ImageMemory, nullptr);
                vkDestroyImage(device, m_Image, nullptr);
                return;
            }
        }

        /// Create Upload Buffer (will be used by SetData)
        {
            VkBufferCreateInfo bufferInfo{};
            bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
            bufferInfo.size = imageSize; // Size for the pixel data
            bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
            bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
            err = vkCreateBuffer(device, &bufferInfo, nullptr, &m_UploadBuffer);
            if (err != VK_SUCCESS)
            {
                KBR_ERROR("Failed to create Vulkan upload buffer (empty tex): {}", VulkanHelpers::VkResultToString(err));
                // Clean up previously allocated resources
                vkDestroySampler(device, m_Sampler, nullptr);
                vkDestroyImageView(device, m_ImageView, nullptr);
                vkFreeMemory(device, m_ImageMemory, nullptr);
                vkDestroyImage(device, m_Image, nullptr);
                return;
            }

            VkMemoryRequirements req;
            vkGetBufferMemoryRequirements(device, m_UploadBuffer, &req);
            VkMemoryAllocateInfo allocInfo{};
            allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            allocInfo.allocationSize = req.size;
            // HOST_COHERENT makes flushes automatic on unmap, or vkFlushMappedMemoryRanges can be manual
            allocInfo.memoryTypeIndex = VulkanHelpers::FindMemoryType(physicalDevice, req.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
            err = vkAllocateMemory(device, &allocInfo, nullptr, &m_UploadBufferMemory);
            if (err != VK_SUCCESS)
            {
                KBR_ERROR("Failed to allocate Vulkan upload buffer memory (empty tex): {}", VulkanHelpers::VkResultToString(err));
                vkDestroyBuffer(device, m_UploadBuffer, nullptr);
                vkDestroySampler(device, m_Sampler, nullptr);
                vkDestroyImageView(device, m_ImageView, nullptr);
                vkFreeMemory(device, m_ImageMemory, nullptr);
                vkDestroyImage(device, m_Image, nullptr);
                return;
            }
            err = vkBindBufferMemory(device, m_UploadBuffer, m_UploadBufferMemory, 0);
            if (err != VK_SUCCESS)
            {
                KBR_ERROR("Failed to bind Vulkan upload buffer memory (empty tex): {}", VulkanHelpers::VkResultToString(err));
                vkFreeMemory(device, m_UploadBufferMemory, nullptr);
                vkDestroyBuffer(device, m_UploadBuffer, nullptr);
                vkDestroySampler(device, m_Sampler, nullptr);
                vkDestroyImageView(device, m_ImageView, nullptr);
                vkFreeMemory(device, m_ImageMemory, nullptr);
                vkDestroyImage(device, m_Image, nullptr);
                return;
            }
        }

        VkCommandBuffer commandBuffer = context.GetOneTimeCommandBuffer();
        Utils::TransitionImageLayout(commandBuffer, m_Image, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        context.SubmitCommandBuffer(commandBuffer);

        m_DescriptorSet = ImGui_ImplVulkan_AddTexture(m_Sampler, m_ImageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	}

	VulkanTexture2D::~VulkanTexture2D()
	{
		CleanupResources();
	}

	uint64_t VulkanTexture2D::GetRendererID() const 
    {
        return reinterpret_cast<ImTextureID>(m_DescriptorSet);
    }

	void VulkanTexture2D::Bind(uint32_t slot) const
	{
        //vkCmdBindDescriptorSets()
	}

	void VulkanTexture2D::SetData(void* data, uint32_t size)
	{
        KBR_PROFILE_FUNCTION();

        KBR_CORE_ASSERT(data, "Data cannot be null when uploading to texture");

        uint32_t expectedSize = m_Spec.Width * m_Spec.Height * 4; // Assuming RGBA8
        KBR_ASSERT(size == expectedSize, "Data size mismatch! Expected {} bytes, got {} bytes.", expectedSize, size);
        if (size != expectedSize || !data)
        {
            KBR_ERROR("Invalid data or size for SetData. Expected size: {}, actual size: {}", expectedSize, size);
            return;
        }

        const VulkanContext& context = VulkanContext::Get();

        /// Copy data to upload buffer
        {
	        const VkDevice device = context.GetDevice();
	        void* map = nullptr;
            VkResult err = vkMapMemory(device, m_UploadBufferMemory, 0, size, 0, &map);
            if (err != VK_SUCCESS)
            {
                KBR_ERROR("Failed to map Vulkan upload buffer memory for SetData: {}", VulkanHelpers::VkResultToString(err));
                return;
            }
            memcpy(map, data, size);
            // If memory is not HOST_COHERENT, flush is needed:
            // VkMappedMemoryRange range{};
            // range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
            // range.memory = m_UploadBufferMemory;
            // range.offset = 0;
            // range.size = size;
            // vkFlushMappedMemoryRanges(device, 1, &range);
            vkUnmapMemory(device, m_UploadBufferMemory);
        }

        const VkCommandBuffer commandBuffer = context.GetOneTimeCommandBuffer();

        /// Transition image from SHADER_READ_ONLY (current) to TRANSFER_DST
        Utils::TransitionImageLayout(commandBuffer, m_Image, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

        /// Copy buffer to image
        VkBufferImageCopy region{};
        region.bufferOffset = 0;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;
        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = 1;
        region.imageOffset = { .x = 0, .y = 0, .z = 0 };
        region.imageExtent = { .width = m_Spec.Width, .height = m_Spec.Height, .depth = 1 };
        vkCmdCopyBufferToImage(commandBuffer, m_UploadBuffer, m_Image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

        /// Transition image from TRANSFER_DST to SHADER_READ_ONLY for sampling
        Utils::TransitionImageLayout(commandBuffer, m_Image, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

        context.SubmitCommandBuffer(commandBuffer);
	}

	void VulkanTexture2D::CleanupResources() const 
    {
		KBR_PROFILE_FUNCTION();

		const VulkanContext& context = VulkanContext::Get();
		const VkDevice device = context.GetDevice();

        vkFreeMemory(device, m_UploadBufferMemory, nullptr);
        vkDestroyBuffer(device, m_UploadBuffer, nullptr);
        vkDestroySampler(device, m_Sampler, nullptr);
        vkDestroyImageView(device, m_ImageView, nullptr);
        vkDestroyImage(device, m_Image, nullptr);
        vkFreeMemory(device, m_ImageMemory, nullptr);
        ImGui_ImplVulkan_RemoveTexture(m_DescriptorSet);
	}
}
