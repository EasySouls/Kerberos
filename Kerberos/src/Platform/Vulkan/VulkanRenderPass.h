#pragma once

#include "Kerberos/Renderer/RenderPass.h"

#include <vulkan/vulkan.h>

namespace Kerberos
{
	class VulkanRenderPass : public RenderPass
	{
	public:
		explicit VulkanRenderPass(const RenderPassSpecification& spec);

	private:
		void CreateRenderPass(const RenderPassSpecification& spec);

		void ReleaseResources();
		void SetDebugName(const std::string& name) const;

	private:
		VkRenderPass m_RenderPass = VK_NULL_HANDLE;
	};

	
}
