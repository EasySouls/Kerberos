#pragma once

#include "Kerberos/Renderer/TextureCube.h"

#include <vulkan/vulkan.h>

namespace Kerberos
{
	class VulkanTextureCube final : public TextureCube
	{
	public:
		using RendererID = uint64_t;

		explicit VulkanTextureCube(const CubemapData& cubemapData);
		~VulkanTextureCube() override;

		void Bind(uint32_t slot = 0) const override;

		RendererID GetRendererID() const override { return m_RendererID; }
		const std::string& GetName() const override { return m_Name; }
		uint32_t GetWidth() const override;
		uint32_t GetHeight() const override;
		const TextureSpecification& GetSpecification() const override { return m_Spec; }

		void SetData(void* data, uint32_t size) override;

		bool operator==(const Texture& other) const override
		{
			return m_RendererID == dynamic_cast<const VulkanTextureCube&>(other).m_RendererID;
		}

		void SetDebugName(const std::string& name) const override;

	private:
		static void AllocateAndBindMemory(VkDevice device, VkPhysicalDevice physicalDevice, VkImage image, VkDeviceMemory& imageMemory);
		static void CreateImage(uint32_t width, uint32_t height, uint8_t mipLevels, VkFormat format, VkImage& image, VkDevice device);
		static void CreateImageView(VkDevice device, VkImageView& imageView, VkImage image, VkFormat format, uint8_t mipLevels);
		static void CreateSampler(VkDevice device, VkPhysicalDevice physicalDevice, uint8_t mipLevels, VkSampler& sampler);
		static void TransitionImageLayout(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels);
		static void CopyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer buffer, VkImage image, const CubemapData& cubemapData);
		static void GenerateMipmaps(VkPhysicalDevice physicalDevice, VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels, uint32_t layerCount);

		void ReleaseResources();

		/// Todo: move this to VulkanHelpers
		static void CreateBuffer(VkDevice device, VkPhysicalDevice physicalDevice, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);

		static VkFormat ToVulkanFormat(const ImageFormat format, const bool isSRGB)
		{
			switch (format)
			{
			case ImageFormat::R8: return VK_FORMAT_R8_UNORM;
			case ImageFormat::RGB8: /*return isSRGB ? VK_FORMAT_R8G8B8_SRGB : VK_FORMAT_R8G8B8_UNORM;*/
			case ImageFormat::RGBA8: return isSRGB ? VK_FORMAT_R8G8B8A8_SRGB : VK_FORMAT_R8G8B8A8_UNORM;
			case ImageFormat::RGBA32F: return VK_FORMAT_R32G32B32A32_SFLOAT;
			case ImageFormat::None: break;
			}

			KBR_CORE_ERROR("Unsupported ImageFormat: {0}", static_cast<int>(format));
			KBR_CORE_ASSERT(false, "Unsupported ImageFormat!");
			return VK_FORMAT_UNDEFINED;
		}

	private:
		RendererID m_RendererID;
		std::string m_Name;
		uint8_t m_MipLevels;
		bool m_SRGB;
		TextureSpecification m_Spec;

		VkImage m_Image = VK_NULL_HANDLE;
		VkImageView m_ImageView = VK_NULL_HANDLE;
		VkSampler m_Sampler = VK_NULL_HANDLE;
		VkDeviceMemory m_ImageMemory = VK_NULL_HANDLE;
		VkFormat m_Format = VK_FORMAT_UNDEFINED;
	};
}


