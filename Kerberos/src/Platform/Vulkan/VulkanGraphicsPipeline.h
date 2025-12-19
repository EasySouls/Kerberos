#pragma once

#include "Kerberos/Renderer/GraphicsPipeline.h"

#include <vulkan/vulkan.h>

namespace Kerberos
{
	class VulkanGraphicsPipeline : public GraphicsPipeline
	{
	public:
		explicit VulkanGraphicsPipeline(PipelineSpecification spec);
		~VulkanGraphicsPipeline() override;

		void Bind() const override;

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