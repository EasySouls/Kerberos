#pragma once

#include <vulkan/vulkan_core.h>

#include "Kerberos/Renderer/Texture.h"

namespace Kerberos
{
	class VulkanTexture2D final : public Texture2D
	{
	public:
		explicit VulkanTexture2D(const std::string& path);
		explicit VulkanTexture2D(const TextureSpecification& spec, Buffer data);
		~VulkanTexture2D() override;

		uint32_t GetWidth() const override { return m_Spec.Width; }
		uint32_t GetHeight() const override { return m_Spec.Height; }
		const TextureSpecification& GetSpecification() const override { return m_Spec; }

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
		TextureSpecification m_Spec;
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

