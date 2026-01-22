#pragma once

#include "Kerberos/Renderer/RenderPass.h"

#include <vulkan/vulkan.h>

#include <unordered_map>
#include <variant>

namespace Kerberos
{
	class VulkanRenderPass : public RenderPass
	{
	public:
		explicit VulkanRenderPass(const RenderPassSpecification& spec);
		~VulkanRenderPass() override = default;

		void SetInput(std::string_view name, const Ref<Texture2D>& image) override;
		void SetInput(std::string_view name, const Ref<TextureCube>& cubeImage) override;
		void SetInput(std::string_view name, const Ref<UniformBuffer>& uniformBuffer) override;

		Ref<Texture2D> GetOutputImage(uint32_t index) const override;

		bool Validate() const override;
		void Bake() override;

	private:
		void CreateRenderPass();

		void ReleaseResources();
		void SetDebugName(const std::string& name) const;

	private:
		std::string m_DebugName;
		Ref<GraphicsPipeline> m_Pipeline;

		using InputType = std::variant<Ref<Texture2D>, Ref<TextureCube>, Ref<UniformBuffer>>;
		std::unordered_map<std::string, InputType> m_Inputs;


		VkRenderPass m_RenderPass = VK_NULL_HANDLE;
	};

	
}
