#include "kbrpch.h"
#include "VulkanPipeline.h"

#include "VulkanContext.h"
#include "VulkanFramebuffer.h"
#include "VulkanShader.h"

namespace Kerberos
{
	VulkanPipeline::VulkanPipeline(PipelineSpecification spec)
		: m_Specification(std::move(spec))
	{
		CreateGraphicsPipeline();
		SetDebugName(m_Specification.Name.empty() ? "VulkanPipeline" : m_Specification.Name);
	}

	VulkanPipeline::~VulkanPipeline() 
	{
		ReleaseResources();
	}

	void VulkanPipeline::CreateGraphicsPipeline() 
	{
		const VulkanShader& shader = m_Specification.Shader->As<VulkanShader>();

		const std::vector<VkDescriptorSetLayout>& descriptorSetLayouts = shader.GetDescriptorSetLayouts();

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
		pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
		pipelineLayoutInfo.pushConstantRangeCount = 0;
		pipelineLayoutInfo.pPushConstantRanges = nullptr; /// TODO

		const VkDevice& device = VulkanContext::Get().GetDevice();

		if (const VkResult result = vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &m_PipelineLayout); result != VK_SUCCESS)
		{
			KBR_CORE_ERROR("Failed to create pipeline layout! {0}", VulkanHelpers::VkResultToString(result));
			KBR_CORE_ASSERT(false, "Failed to create pipeline layout! {0}", VulkanHelpers::VkResultToString(result));
		}

		const auto& shaderModules = shader.GetShaderModules();
		const VkShaderModule vertShaderModule = shaderModules.at(VK_SHADER_STAGE_VERTEX_BIT);
		const VkShaderModule fragShaderModule = shaderModules.at(VK_SHADER_STAGE_FRAGMENT_BIT);

		VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
		vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vertShaderStageInfo.module = vertShaderModule;
		vertShaderStageInfo.pName = "main";

		VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
		fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		fragShaderStageInfo.module = fragShaderModule;
		fragShaderStageInfo.pName = "main";

		VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

		const std::vector<VkDynamicState> dynamicStates = {
			VK_DYNAMIC_STATE_VIEWPORT,
			VK_DYNAMIC_STATE_SCISSOR
		};

		VkPipelineDynamicStateCreateInfo dynamicState{};
		dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
		dynamicState.pDynamicStates = dynamicStates.data();

		const BufferLayout& vertexLayout = m_Specification.Layout;

		VkVertexInputBindingDescription vertexBindingDescription{};
		vertexBindingDescription.binding = 0;
		vertexBindingDescription.stride = vertexLayout.GetStride();
		vertexBindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		const uint32_t attributeCount = static_cast<uint32_t>(vertexLayout.GetElements().size());
		std::vector<VkVertexInputAttributeDescription> vertexAttributeDescriptions;
		for (uint32_t i = 0; i < attributeCount; ++i)
		{
			const BufferElement& element = vertexLayout.GetElements()[i];

			VkVertexInputAttributeDescription attributeDescription{};
			attributeDescription.binding = 0;
			attributeDescription.location = i;
			attributeDescription.format = VulkanHelpers::ShaderDataTypeToVulkanFormat(element.Type);
			attributeDescription.offset = element.Offset;
			vertexAttributeDescriptions.push_back(attributeDescription);
		}

		VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputInfo.vertexBindingDescriptionCount = 1;
		vertexInputInfo.pVertexBindingDescriptions = &vertexBindingDescription;
		vertexInputInfo.vertexAttributeDescriptionCount = attributeCount;
		vertexInputInfo.pVertexAttributeDescriptions = vertexAttributeDescriptions.data();

		VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
		inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssembly.topology = VulkanHelpers::GetVulkanPrimitiveTopology(m_Specification.PrimitiveTopology);
		inputAssembly.primitiveRestartEnable = VK_FALSE;

		VkPipelineViewportStateCreateInfo viewportState{};
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount = 1;
		viewportState.scissorCount = 1;

		VkPipelineRasterizationStateCreateInfo rasterizer{};
		rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizer.depthClampEnable = VK_FALSE;
		rasterizer.rasterizerDiscardEnable = VK_FALSE;
		rasterizer.polygonMode = m_Specification.Wireframe ? VK_POLYGON_MODE_LINE : VK_POLYGON_MODE_FILL;
		rasterizer.lineWidth = 1.0f;
		rasterizer.cullMode = VulkanHelpers::GetVulkanCullMode(m_Specification.CullMode);
		rasterizer.frontFace = VulkanHelpers::GetVulkanFrontFace(m_Specification.FrontFace);
		rasterizer.depthBiasEnable = VK_FALSE;
		rasterizer.depthBiasConstantFactor = 0.0f;
		rasterizer.depthBiasClamp = 0.0f;
		rasterizer.depthBiasSlopeFactor = 0.0f;

		VkPipelineMultisampleStateCreateInfo multisampling{};
		multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampling.sampleShadingEnable = VK_FALSE;
		multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		multisampling.minSampleShading = 1.0f;
		multisampling.pSampleMask = nullptr;
		multisampling.alphaToCoverageEnable = VK_FALSE;
		multisampling.alphaToOneEnable = VK_FALSE;

		VkPipelineDepthStencilStateCreateInfo depthStencil{};
		depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depthStencil.depthTestEnable = m_Specification.DepthTest != DepthTest::None ? VK_TRUE : VK_FALSE;
		depthStencil.depthWriteEnable = m_Specification.DepthTest != DepthTest::None ? VK_TRUE : VK_FALSE; /// TODO: See if we need to disable depth write for some cases
		depthStencil.depthCompareOp = VulkanHelpers::GetVulkanDepthCompareOp(m_Specification.DepthTest);
		depthStencil.stencilTestEnable = VK_FALSE;	/// TODO: See if we need to configure stencil testing
		depthStencil.depthBoundsTestEnable = VK_FALSE;


		VkPipelineColorBlendAttachmentState colorBlendAttachment{};
		colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachment.blendEnable = VK_FALSE;
		colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
		colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
		colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
		colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
		colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
		colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional

		VkPipelineColorBlendStateCreateInfo colorBlending{};
		colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlending.logicOpEnable = VK_FALSE;
		colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
		colorBlending.attachmentCount = 1;
		colorBlending.pAttachments = &colorBlendAttachment;
		colorBlending.blendConstants[0] = 0.0f; // Optional
		colorBlending.blendConstants[1] = 0.0f; // Optional
		colorBlending.blendConstants[2] = 0.0f; // Optional
		colorBlending.blendConstants[3] = 0.0f; // Optional

		const VulkanFramebuffer& framebuffer = m_Specification.TargetFramebuffer->As<VulkanFramebuffer>();

		VkGraphicsPipelineCreateInfo pipelineInfo{};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount = 2;
		pipelineInfo.pStages = shaderStages;
		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &inputAssembly;
		pipelineInfo.pViewportState = &viewportState;
		pipelineInfo.pRasterizationState = &rasterizer;
		pipelineInfo.pMultisampleState = &multisampling;
		pipelineInfo.pDepthStencilState = &depthStencil;
		pipelineInfo.pColorBlendState = &colorBlending;
		pipelineInfo.pDynamicState = &dynamicState;
		pipelineInfo.layout = m_PipelineLayout;
		pipelineInfo.renderPass = framebuffer.GetRenderPass(); /// TODO: Get the information about the renderpass
		pipelineInfo.subpass = 0;

		if (const VkResult result = vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_Pipeline); result != VK_SUCCESS)
		{
			KBR_CORE_ERROR("Failed to create graphics pipeline! {0}", VulkanHelpers::VkResultToString(result));
			KBR_CORE_ASSERT(false, "Failed to create graphics pipeline! {0}", VulkanHelpers::VkResultToString(result));
		}
	}

	void VulkanPipeline::ReleaseResources() const 
	{
		const VkDevice& device = VulkanContext::Get().GetDevice();

		vkDestroyPipeline(device, m_Pipeline, nullptr);
		vkDestroyPipelineLayout(device, m_PipelineLayout, nullptr);
	}

	void VulkanPipeline::SetDebugName(const std::string& name) const 
	{
		KBR_CORE_ASSERT(!name.empty(), "Pipeline name is empty!");
		KBR_CORE_ASSERT(m_Pipeline != VK_NULL_HANDLE, "Pipeline is null!");
		KBR_CORE_ASSERT(m_PipelineLayout != VK_NULL_HANDLE, "Pipeline Layout is null!");

		VulkanHelpers::SetObjectDebugName(VulkanContext::Get().GetDevice(), VK_OBJECT_TYPE_PIPELINE, reinterpret_cast<uint64_t>(m_Pipeline), name);
		VulkanHelpers::SetObjectDebugName(VulkanContext::Get().GetDevice(), VK_OBJECT_TYPE_PIPELINE_LAYOUT, reinterpret_cast<uint64_t>(m_PipelineLayout), name + " Layout");
	}
}
