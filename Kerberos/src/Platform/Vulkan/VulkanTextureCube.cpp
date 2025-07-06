#include "kbrpch.h"
#include "VulkanTextureCube.h"

#include "VulkanContext.h"
#include "VulkanHelpers.h"

namespace Kerberos
{
	VulkanTextureCube::VulkanTextureCube(const CubemapData& data) 
	{
		throw std::runtime_error("VulkanTextureCube::VulkanTextureCube(const CubemapData& data) is not yet implemented.");
	}

	VulkanTextureCube::~VulkanTextureCube()
	{

	}

	void VulkanTextureCube::Bind(uint32_t slot) const
	{

	}

	uint32_t VulkanTextureCube::GetWidth() const
	{
		throw std::runtime_error("VulkanTextureCube::GetWidth() is not yet implemented.");
	}

	uint32_t VulkanTextureCube::GetHeight() const
	{
		throw std::runtime_error("VulkanTextureCube::GetWidth() is not yet implemented.");
	}

	void VulkanTextureCube::SetData(void* data, uint32_t size)
	{
		
	}

	void VulkanTextureCube::SetDebugName(const std::string& name) const 
	{
		const VkDevice& device = VulkanContext::Get().GetDevice();
		/*VulkanHelpers::SetObjectDebugName(device, VK_OBJECT_TYPE_IMAGE, reinterpret_cast<uint64_t>(m_Image), name + " Image");
		VulkanHelpers::SetObjectDebugName(device, VK_OBJECT_TYPE_IMAGE_VIEW, reinterpret_cast<uint64_t>(m_ImageView), name + " Image View");
		VulkanHelpers::SetObjectDebugName(device, VK_OBJECT_TYPE_SAMPLER, reinterpret_cast<uint64_t>(m_Sampler), name + " Sampler");*/
		VulkanHelpers::SetObjectDebugName(device, VK_OBJECT_TYPE_DESCRIPTOR_SET, m_RendererID, name + " Descriptor Set");
	}
}
