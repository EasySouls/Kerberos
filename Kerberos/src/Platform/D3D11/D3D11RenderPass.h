#pragma once
#include "Kerberos/Renderer/RenderPass.h"

namespace Kerberos 
{
	class D3D11RenderPass : public RenderPass
	{
	public:
		explicit D3D11RenderPass(const RenderPassSpecification& spec);
		~D3D11RenderPass() override;

		void SetInput(std::string_view name, const Ref<Texture2D>& texture) override;
		void SetInput(std::string_view name, const Ref<UniformBuffer>& uniformBuffer) override;
		Ref<Texture2D> GetOutputImage(uint32_t index) const override;
		bool Validate() const override;
		void Bake() override;
	};
}