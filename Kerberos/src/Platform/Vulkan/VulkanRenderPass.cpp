#include "kbrpch.h"
#include "VulkanRenderPass.h"

#include "VulkanContext.h"

namespace Kerberos
{
	VulkanRenderPass::VulkanRenderPass(const RenderPassSpecification& spec) 
	{
		CreateRenderPass(spec);

		SetDebugName(spec.Name.empty() ? "VulkanRenderPass" : spec.Name);
	}

	void VulkanRenderPass::CreateRenderPass(const RenderPassSpecification& spec) 
	{
		const VkDevice& device = VulkanContext::Get().GetDevice();

		VkAttachmentDescription colorAttachment{};
		colorAttachment.format = VK_FORMAT_R8G8B8_SRGB; // TODO: swapChainImageFormat or other format based on attachments;
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR; /// TODO: Customize with the spec
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE; /// TODO: Customize with the spec
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; /// TODO: Customize with the spec

		VkAttachmentReference colorAttachmentRef;
		colorAttachmentRef.attachment = 0; /// Index of the attachment in the attachment descriptions array
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		/// The index of the attachment in this array is directly referenced from the 
		/// fragment shader with the layout(location = 0) out vec4 outColor directive!
		subpass.pColorAttachments = &colorAttachmentRef;

		VkRenderPassCreateInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = 1;
		renderPassInfo.pAttachments = &colorAttachment;
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;

		if (const VkResult result = vkCreateRenderPass(device, &renderPassInfo, nullptr, &m_RenderPass); result != VK_SUCCESS)
		{
			KBR_CORE_ERROR("Failed to create render pass: {}", VulkanHelpers::VkResultToString(result));
			KBR_CORE_ASSERT(false, "Failed to create render pass: {}", VulkanHelpers::VkResultToString(result));
		}
	}

	void VulkanRenderPass::ReleaseResources() 
	{
		const VkDevice& device = VulkanContext::Get().GetDevice();

		if (m_RenderPass != VK_NULL_HANDLE)
		{
			vkDestroyRenderPass(device, m_RenderPass, nullptr);
			m_RenderPass = VK_NULL_HANDLE;
		}
	}

	void VulkanRenderPass::SetDebugName(const std::string& name) const 
	{
		KBR_CORE_ASSERT(!name.empty(), "RenderPass name is empty!");
		KBR_CORE_ASSERT(m_RenderPass != VK_NULL_HANDLE, "RenderPass is null!");

		VulkanHelpers::SetObjectDebugName(VulkanContext::Get().GetDevice(), VK_OBJECT_TYPE_RENDER_PASS, reinterpret_cast<uint64_t>(m_RenderPass), name);
	}

	void VulkanRenderPass::SetInput(std::string_view name, const Ref<Texture2D>& image) {}
	Ref<Texture2D> VulkanRenderPass::GetOutputImage(uint32_t index) const {}
	bool VulkanRenderPass::Validate() const {}
	void VulkanRenderPass::Bake() {}
}
