#pragma once

#include "Kerberos/Renderer/GraphicsPipeline.h"

#include <d3d11.h>
#include <wrl/client.h>

namespace Kerberos
{
	class D3D11GraphicsPipeline final : public GraphicsPipeline
	{
	public:
		explicit D3D11GraphicsPipeline(const PipelineSpecification& spec);
		~D3D11GraphicsPipeline() override;

		void Bind() const override;

		const PipelineSpecification& GetSpecification() const override { return m_Specification; }
	private:
		PipelineSpecification m_Specification;

		Microsoft::WRL::ComPtr<ID3D11InputLayout> m_InputLayout;
		D3D11_PRIMITIVE_TOPOLOGY m_PrimitiveTopology;
		Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_RasterizerState;
		Microsoft::WRL::ComPtr<ID3D11DepthStencilState> m_DepthStencilState;
		Microsoft::WRL::ComPtr<ID3D11BlendState> m_BlendState;
	};
}
