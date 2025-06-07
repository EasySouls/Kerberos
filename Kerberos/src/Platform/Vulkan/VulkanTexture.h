#pragma once

#include <vulkan/vulkan_core.h>

#include "Kerberos/Renderer/Texture.h"

namespace Kerberos
{
	class VulkanTexture2D final : public Texture2D
	{
	public:
		explicit VulkanTexture2D(const std::string& path);
		explicit VulkanTexture2D(uint32_t width, uint32_t height);
		~VulkanTexture2D() override;

		uint32_t GetWidth() const override { return m_Width; }
		uint32_t GetHeight() const override { return m_Height; }

		uint64_t GetRendererID() const override;

		void Bind(uint32_t slot = 0) const override;

		void SetData(void* data, uint32_t size) override;

		bool operator==(const Texture& other) const override
		{
			/// TODO: Test if this works
			return m_DescriptorSet == dynamic_cast<const VulkanTexture2D&>(other).m_DescriptorSet;
		}

	private:
		void CleanupResources() const;

	private:
		uint32_t m_Width = 0;
		uint32_t m_Height = 0;
		std::string m_Path;

		VkImage         m_Image;
		VkImageView     m_ImageView;
		VkDeviceMemory  m_ImageMemory;
		VkSampler       m_Sampler;
		VkBuffer        m_UploadBuffer;
		VkDeviceMemory  m_UploadBufferMemory;
		VkDescriptorSet m_DescriptorSet;
	};
}

