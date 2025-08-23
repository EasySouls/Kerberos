#include "kbrpch.h"
#include "VulkanTextureCube.h"

#include "VulkanContext.h"
#include "VulkanHelpers.h"

#include <backends/imgui_impl_vulkan.h>

#include <algorithm>
#include <cmath>
#include <utility>

namespace Kerberos
{
	VulkanTextureCube::VulkanTextureCube(const CubemapData& cubemapData) 
	{
		const auto& firstFace = cubemapData.Faces[0].Specification;
		const uint32_t width = firstFace.Width;
		const uint32_t height = firstFace.Height;
		const ImageFormat format = firstFace.Format;

		KBR_CORE_ASSERT(cubemapData.Faces.size() == 6, "Cubemap must have 6 faces!");
		KBR_CORE_ASSERT(format != ImageFormat::None, "Cubemap faces must have a valid format!");

		for (size_t i = 1; i < 6; ++i)
		{
			const auto& faceSpec = cubemapData.Faces[i].Specification;
			if (faceSpec.Width != width || faceSpec.Height != height || faceSpec.Format != format)
			{
				KBR_CORE_ERROR("All cubemap faces must have the same dimensions and format! Face {0} has different dimensions or format.", i);
				KBR_CORE_ASSERT(false, "All cubemap faces must have the same dimensions and format!");
			}
		}

		/// Todo: use std::max
		m_MipLevels = firstFace.GenerateMips ? static_cast<uint32_t>(std::floor(std::log2(max(width, height)))) + 1 : 1;
		m_Format = ToVulkanFormat(format, cubemapData.IsSRGB);

		VkDeviceSize totalSize = 0;
		for (const auto& [Specification, Buffer] : cubemapData.Faces)
		{
			totalSize += Buffer.Size;
		}

		const VkDevice& device = VulkanContext::Get().GetDevice();
		const VkPhysicalDevice& physicalDevice = VulkanContext::Get().GetPhysicalDevice();

		/// Usually we create the staging buffer first, then create the image and allocate memory for it.
		/// But here we need the image to get the memory requirements for the image, so we create it first.
		/// This is needed for the case where the image format is RGB8, but we convert it to RGBA8 for Vulkan,
		/// but if we allocate memory only based on the buffer size, we might not allocate enough memory for the image.

		CreateImage(width, height, m_MipLevels, m_Format, m_Image, device);

		const VkDeviceSize allocatedSize = AllocateAndBindMemory(device, physicalDevice, m_Image, m_ImageMemory);

		KBR_CORE_ASSERT(allocatedSize >= totalSize, "Allocated image memory size ({0}) is less than total data size ({1})", allocatedSize, totalSize);

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		VkDeviceSize stagingBufferSize = CreateBufferAndGetSize(device, physicalDevice, allocatedSize,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			stagingBuffer, stagingBufferMemory);

		KBR_CORE_ASSERT(totalSize <= stagingBufferSize, "Data size ({0}) exceeds allocated buffer size ({1})", totalSize, stagingBufferSize);

		/// Map and copy all face data into the staging buffer
		void* data;
		{
			vkMapMemory(device, stagingBufferMemory, 0, totalSize, 0, &data);
			char* pData = static_cast<char*>(data);
			VkDeviceSize currentOffset = 0;
			for (const auto& [Specification, Buffer] : cubemapData.Faces)
			{
				memcpy(pData + currentOffset, Buffer.Data, Buffer.Size);
				currentOffset += Buffer.Size;
			}
			vkUnmapMemory(device, stagingBufferMemory);
		}

		const VkCommandBuffer cmdBuffer = VulkanContext::Get().GetOneTimeCommandBuffer();

		/// Transition image layout to be ready for copying
		TransitionImageLayout(cmdBuffer, m_Image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, m_MipLevels);

		/// Copy data from staging buffer to image
		CopyBufferToImage(cmdBuffer, stagingBuffer, m_Image, { .width = width, .height = height, .depth = 1 });

		if (!firstFace.GenerateMips)
		{
			TransitionImageLayout(cmdBuffer, m_Image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, m_MipLevels);
		}

		VulkanContext::Get().SubmitCommandBuffer(cmdBuffer);

		if (firstFace.GenerateMips)
		{
			GenerateMipmaps(physicalDevice, m_Image, m_Format, static_cast<int32_t>(width), static_cast<int32_t>(height), m_MipLevels, 6);
		}

		/// Cleanup staging buffer
		vkDestroyBuffer(device, stagingBuffer, nullptr);
		vkFreeMemory(device, stagingBufferMemory, nullptr);

		CreateImageView(device, m_ImageView, m_Image, m_Format, m_MipLevels);

		CreateSampler(device, physicalDevice, m_MipLevels, m_Sampler);

		m_DescriptorSet = ImGui_ImplVulkan_AddTexture(m_Sampler, m_ImageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	}

	VulkanTextureCube::~VulkanTextureCube()
	{
		ReleaseResources();
	}

	void VulkanTextureCube::Bind(uint32_t slot) const
	{

	}

	VulkanTextureCube::RendererID VulkanTextureCube::GetRendererID() const 
	{
		return reinterpret_cast<ImTextureID>(m_DescriptorSet);
	}

	uint32_t VulkanTextureCube::GetWidth() const
	{
		return m_Spec.Width;
	}

	uint32_t VulkanTextureCube::GetHeight() const
	{
		return m_Spec.Height;
	}

	void VulkanTextureCube::SetData(void* data, uint32_t size)
	{
		// TODO: Implement data upload for cubemap faces
	}

	void VulkanTextureCube::SetDebugName(const std::string& name) const 
	{
		const VkDevice& device = VulkanContext::Get().GetDevice();
		VulkanHelpers::SetObjectDebugName(device, VK_OBJECT_TYPE_IMAGE, reinterpret_cast<uint64_t>(m_Image), name + " Image");
		VulkanHelpers::SetObjectDebugName(device, VK_OBJECT_TYPE_IMAGE_VIEW, reinterpret_cast<uint64_t>(m_ImageView), name + " Image View");
		VulkanHelpers::SetObjectDebugName(device, VK_OBJECT_TYPE_SAMPLER, reinterpret_cast<uint64_t>(m_Sampler), name + " Sampler");
		VulkanHelpers::SetObjectDebugName(device, VK_OBJECT_TYPE_DESCRIPTOR_SET, reinterpret_cast<uint64_t>(m_DescriptorSet), name + " Descriptor Set");
	}

	void VulkanTextureCube::CreateSampler(const VkDevice device, const VkPhysicalDevice physicalDevice, const uint8_t mipLevels, VkSampler& sampler) 
	{
		VkSamplerCreateInfo samplerInfo{};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = VK_FILTER_LINEAR;
		samplerInfo.minFilter = VK_FILTER_LINEAR;
		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;

		VkPhysicalDeviceFeatures deviceFeatures{};
		vkGetPhysicalDeviceFeatures(physicalDevice, &deviceFeatures);
		samplerInfo.anisotropyEnable = deviceFeatures.samplerAnisotropy;

		VkPhysicalDeviceProperties properties{};
		vkGetPhysicalDeviceProperties(physicalDevice, &properties);
		samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;

		samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		samplerInfo.unnormalizedCoordinates = VK_FALSE;
		samplerInfo.compareEnable = VK_FALSE;
		samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerInfo.mipLodBias = 0.0f;
		samplerInfo.minLod = 0.0f;
		samplerInfo.maxLod = mipLevels > 1 ? static_cast<float>(mipLevels - 1) : 0.0f;

		if (vkCreateSampler(device, &samplerInfo, nullptr, &sampler) != VK_SUCCESS)
		{
			KBR_CORE_ERROR("Failed to create texture sampler!");
			KBR_CORE_ASSERT(false, "Failed to create texture sampler!");
		}
	}

	void VulkanTextureCube::CreateImage(const uint32_t width, const uint32_t height, const uint8_t mipLevels, const VkFormat format, VkImage& image, const VkDevice device) 
	{
		constexpr VkFormatFeatureFlags requiredFeatures = VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT | VK_FORMAT_FEATURE_TRANSFER_SRC_BIT | VK_FORMAT_FEATURE_TRANSFER_DST_BIT;
		constexpr VkImageTiling imageTiling = VK_IMAGE_TILING_OPTIMAL;

		if (!VulkanHelpers::IsFormatSupported(VulkanContext::Get().GetPhysicalDevice(), format, imageTiling, requiredFeatures))
		{
			KBR_CORE_ASSERT(false, "Current VkFormat is not supported with the required format features!");
		}

		VkImageCreateInfo imageInfo{};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.extent.width = width;
		imageInfo.extent.height = height;
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = mipLevels;
		imageInfo.arrayLayers = 6; /// Crucial for cubemaps
		imageInfo.format = format;
		imageInfo.tiling = imageTiling;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;

		if (vkCreateImage(device, &imageInfo, nullptr, &image) != VK_SUCCESS)
		{
			KBR_CORE_ERROR("Failed to create image!");
			KBR_CORE_ASSERT(false, "Failed to create image!");
		}
	}

	void VulkanTextureCube::CreateImageView(const VkDevice device, VkImageView& imageView, const VkImage image, const VkFormat format, const uint8_t mipLevels) 
	{
		VkImageViewCreateInfo viewInfo{};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = image;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
		viewInfo.format = format;
		viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = mipLevels;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 6;

		if (vkCreateImageView(device, &viewInfo, nullptr, &imageView) != VK_SUCCESS)
		{
			KBR_CORE_ERROR("Failed to create texture cube image view!");
			KBR_CORE_ASSERT(false, "Failed to create texture cube image view!");
		}
	}

	void VulkanTextureCube::TransitionImageLayout(const VkCommandBuffer commandBuffer, const VkImage image, const VkImageLayout oldLayout, const VkImageLayout newLayout, const uint32_t mipLevels) 
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
		barrier.subresourceRange.levelCount = mipLevels;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 6;

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
		else
		{
			KBR_CORE_ASSERT(false, "Unsupported image layout");
			return;
		}

		vkCmdPipelineBarrier(commandBuffer, sourceStage, destinationStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);
	}

	void VulkanTextureCube::CopyBufferToImage(const VkCommandBuffer commandBuffer, const VkBuffer buffer, const VkImage image, const VkExtent3D imageExtent) 
	{
		VkBufferImageCopy region;
		region.bufferOffset = 0;
		region.bufferRowLength = 0;
		region.bufferImageHeight = 0;
		region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		region.imageSubresource.mipLevel = 0;
		region.imageSubresource.baseArrayLayer = 0;
		region.imageSubresource.layerCount = 6;
		region.imageOffset = {.x = 0, .y = 0, .z = 0 };
		region.imageExtent = imageExtent;

		vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
	}

	VkDeviceSize VulkanTextureCube::AllocateAndBindMemory(const VkDevice device, const VkPhysicalDevice physicalDevice, const VkImage image, VkDeviceMemory& imageMemory) 
	{
		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements(device, image, &memRequirements);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = VulkanHelpers::FindMemoryType(physicalDevice, memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		if (const VkResult result = vkAllocateMemory(device, &allocInfo, nullptr, &imageMemory); result != VK_SUCCESS)
		{
			KBR_CORE_ASSERT(false, "Failed to allocate image memory: {}", VulkanHelpers::VkResultToString(result));
		}

		vkBindImageMemory(device, image, imageMemory, 0);

		return memRequirements.size;
	}

	void VulkanTextureCube::ReleaseResources() 
	{
		const VkDevice& device = VulkanContext::Get().GetDevice();

		vkDeviceWaitIdle(device);

		if (m_ImageView != VK_NULL_HANDLE)
		{
			vkDestroyImageView(device, m_ImageView, nullptr);
			m_ImageView = VK_NULL_HANDLE;
		}

		if (m_Sampler != VK_NULL_HANDLE)
		{
			vkDestroySampler(device, m_Sampler, nullptr);
			m_Sampler = VK_NULL_HANDLE;
		}

		if (m_Image != VK_NULL_HANDLE)
		{
			vkDestroyImage(device, m_Image, nullptr);
			m_Image = VK_NULL_HANDLE;
		}

		if (m_ImageMemory != VK_NULL_HANDLE)
		{
			vkFreeMemory(device, m_ImageMemory, nullptr);
			m_ImageMemory = VK_NULL_HANDLE;
		}

		if (m_DescriptorSet != VK_NULL_HANDLE)
		{
			ImGui_ImplVulkan_RemoveTexture(m_DescriptorSet);
			m_DescriptorSet = VK_NULL_HANDLE;
		}
	}

	void VulkanTextureCube::GenerateMipmaps(const VkPhysicalDevice physicalDevice, const VkImage image, const VkFormat imageFormat, const int32_t texWidth, const int32_t texHeight, const uint32_t mipLevels, const uint32_t layerCount)
	{
		/// Check if image format supports linear blitting
		VkFormatProperties formatProperties;
		vkGetPhysicalDeviceFormatProperties(physicalDevice, imageFormat, &formatProperties);
		if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT))
		{
			KBR_CORE_ERROR("Texture image format doesn't support linear blitting");
			KBR_CORE_ASSERT(false, "Texture image format doesn't support linear blitting");
			return;
		}

		const VkCommandBuffer commandBuffer = VulkanContext::Get().GetOneTimeCommandBuffer();

		VkImageMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.image = image;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = layerCount;
		barrier.subresourceRange.levelCount = 1;

		int32_t mipWidth = texWidth;
		int32_t mipHeight = texHeight;

		for (uint32_t i = 1; i < mipLevels; i++)
		{
			/// Transition the previous mip level to be a transfer source
			barrier.subresourceRange.baseMipLevel = i - 1;
			barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

			vkCmdPipelineBarrier(commandBuffer,
				VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
				0, nullptr, 0, nullptr, 1, &barrier);

			VkImageBlit blit{};
			blit.srcOffsets[0] = { .x = 0, .y = 0, .z = 0 };
			blit.srcOffsets[1] = { .x = mipWidth, .y = mipHeight, .z = 1 };
			blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			blit.srcSubresource.mipLevel = i - 1;
			blit.srcSubresource.baseArrayLayer = 0;
			blit.srcSubresource.layerCount = layerCount;
			blit.dstOffsets[0] = {.x = 0, .y = 0, .z = 0 };
			blit.dstOffsets[1] = {
					.x = mipWidth > 1 ? mipWidth / 2 : 1, 
					.y = mipHeight > 1 ? mipHeight / 2 : 1, 
					.z = 1 
			};
			blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			blit.dstSubresource.mipLevel = i;
			blit.dstSubresource.baseArrayLayer = 0;
			blit.dstSubresource.layerCount = layerCount;

			vkCmdBlitImage(commandBuffer,
				image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				1, &blit, VK_FILTER_LINEAR);

			/// Transition the previous mip level to shader-read-only
			barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
			barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			vkCmdPipelineBarrier(commandBuffer,
				VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
				0, nullptr, 0, nullptr, 1, &barrier);

			if (mipWidth > 1) mipWidth /= 2;
			if (mipHeight > 1) mipHeight /= 2;
		}

		/// Transition the last mip level to shader-read-only
		barrier.subresourceRange.baseMipLevel = mipLevels - 1;
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		vkCmdPipelineBarrier(commandBuffer,
			VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
			0, nullptr, 0, nullptr, 1, &barrier);

		VulkanContext::Get().SubmitCommandBuffer(commandBuffer);
	}

	VkDeviceSize VulkanTextureCube::CreateBufferAndGetSize(const VkDevice device, const VkPhysicalDevice physicalDevice, const VkDeviceSize requestedSize, const VkBufferUsageFlags usage, const VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory)
	{
		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = requestedSize;
		bufferInfo.usage = usage;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		if (const VkResult result = vkCreateBuffer(device, &bufferInfo, nullptr, &buffer); result != VK_SUCCESS)
		{
			KBR_CORE_ASSERT(false, "Failed to create buffer: {}", VulkanHelpers::VkResultToString(result));
		}

		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = VulkanHelpers::FindMemoryType(physicalDevice, memRequirements.memoryTypeBits, properties);

		if (const VkResult result = vkAllocateMemory(device, &allocInfo, nullptr, &bufferMemory); result != VK_SUCCESS)
		{
			KBR_CORE_ASSERT(false, "Failed to allocate buffer memory: {}", VulkanHelpers::VkResultToString(result));
		}

		vkBindBufferMemory(device, buffer, bufferMemory, 0);

		return memRequirements.size;
	}
}
