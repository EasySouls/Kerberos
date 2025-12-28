#pragma once

#include "Kerberos/Renderer/RenderPass.h"

#include <vulkan/vulkan.h>

namespace Kerberos
{
	class VulkanRenderPass : public RenderPass
	{
	public:
		explicit VulkanRenderPass(const RenderPassSpecification& spec);
		~VulkanRenderPass() override = default;

		void SetInput(std::string_view name, const Ref<UniformBuffer>& uniformBuffer) override;
		void SetInput(std::string_view name, const Ref<Texture2D>& image) override;
		Ref<Texture2D> GetOutputImage(uint32_t index) const override;
		bool Validate() const override;
		void Bake() override;

	private:
		void CreateRenderPass(const RenderPassSpecification& spec);

		void ReleaseResources();
		void SetDebugName(const std::string& name) const;

	private:
		VkRenderPass m_RenderPass = VK_NULL_HANDLE;
	};

	
}
