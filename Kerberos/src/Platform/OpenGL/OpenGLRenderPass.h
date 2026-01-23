#pragma once
#include "Kerberos/Renderer/RenderPass.h"

namespace Kerberos
{
	class OpenGLRenderPass : public RenderPass
	{
	public:
		explicit OpenGLRenderPass(RenderPassSpecification spec);
		~OpenGLRenderPass() override = default;

		void Begin() override;
		void End() override;

		void SetInput(std::string_view name, const Ref<Texture2D>& texture) override;
		void SetInput(std::string_view name, const Ref<UniformBuffer>& uniformBuffer) override;
		void SetInput(std::string_view name, const Ref<TextureCube>& texture) override;

		Ref<Texture2D> GetOutputImage(uint32_t index) const override;
		Ref<Texture2D> GetOutputDepthImage() const override;

		bool Validate() const override;
		void Bake() override;

	private:
		RenderPassSpecification m_Specification;
	};
}