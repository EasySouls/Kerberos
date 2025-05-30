#include "kbrpch.h"
#include "VulkanTextureCube.h"

namespace Kerberos
{
	VulkanTextureCube::VulkanTextureCube(std::string name, const std::vector<std::string>& faces, bool generateMipmaps,
		bool srgb)
			: m_Name(std::move(name))
	{

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
}
