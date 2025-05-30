#include "kbrpch.h"
#include "VulkanFramebuffer.h"

namespace Kerberos
{
	VulkanFramebuffer::VulkanFramebuffer(const FramebufferSpecification& spec)
		: m_Specification(spec)
	{

	}

	void VulkanFramebuffer::Invalidate()
	{

	}

	void VulkanFramebuffer::Bind()
	{

	}

	void VulkanFramebuffer::Unbind()
	{

	}

	void VulkanFramebuffer::Resize(uint32_t width, uint32_t height)
	{

	}

	uint64_t VulkanFramebuffer::GetColorAttachmentRendererID(uint32_t index) const
	{
		return 0; // Placeholder, should return the renderer ID of the color attachment at the specified index
	}

	void VulkanFramebuffer::ReleaseResources() const
	{
		
	}
}
