#pragma once

#include "Kerberos/Renderer/Pipeline.h"

#include <vulkan/vulkan.h>

namespace Kerberos
{
	class VulkanPipeline : public Pipeline
	{
	public:
		explicit VulkanPipeline(PipelineSpecification spec);
		~VulkanPipeline() override;

		const PipelineSpecification& GetSpecification() const override { return m_Specification; }
	private:
		void CreateGraphicsPipeline();
		void ReleaseResources() const;

		void SetDebugName(const std::string& name) const;
	private:
		PipelineSpecification m_Specification;

		VkPipeline m_Pipeline = VK_NULL_HANDLE;
		VkPipelineLayout m_PipelineLayout = VK_NULL_HANDLE;
	};
}