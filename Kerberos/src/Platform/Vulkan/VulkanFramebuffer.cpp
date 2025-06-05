#include "kbrpch.h"
#include "VulkanFramebuffer.h"

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
