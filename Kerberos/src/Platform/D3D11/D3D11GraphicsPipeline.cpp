#include "kbrpch.h"
#include "D3D11GraphicsPipeline.h"

#include "D3D11Context.h"
#include "D3D11Shader.h"
#include "Utils/VertexUtils.h"


namespace Kerberos
{
	namespace
	{
		D3D11_PRIMITIVE_TOPOLOGY ConvertPrimitiveTopology(const GraphicsPipeline::Topology topology)
		{
			switch (topology)
			{
			case GraphicsPipeline::Topology::Triangles:   return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
			case GraphicsPipeline::Topology::Lines:       return D3D11_PRIMITIVE_TOPOLOGY_LINELIST;
			case GraphicsPipeline::Topology::LineStrip:   return D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP;
			}
			KBR_CORE_ASSERT(false, "Unknown Pipeline::Topology!");
			return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		}
	}

	D3D11GraphicsPipeline::D3D11GraphicsPipeline(const PipelineSpecification& spec)
		: m_Specification(spec)
	{
		const auto& device = D3D11Context::Get().GetDevice();

		const D3D11Shader& shader = spec.Shader->As<D3D11Shader>();
		const auto& vsBytecode = shader.GetVertexShaderBlob();
		const auto& reflection = shader.GetReflectionData();
		const auto& elements = VertexUtils::CreateInputLayoutFromVertexInputs(reflection.vertexInputs);

		HRESULT hr = device->CreateInputLayout(
			elements.data(),
			static_cast<uint32_t>(elements.size()),
			vsBytecode->GetBufferPointer(),
			vsBytecode->GetBufferSize(),
			m_InputLayout.GetAddressOf()
		);

		if (FAILED(hr))
		{
			KBR_CORE_ERROR("Failed to create input layout for pipeline!");
			KBR_CORE_ASSERT(false, "Failed to create input layout for pipeline!");
		}

		m_PrimitiveTopology = ConvertPrimitiveTopology(spec.PrimitiveTopology);

		D3D11_RASTERIZER_DESC rs{};
		rs.FillMode = spec.Wireframe ? D3D11_FILL_WIREFRAME : D3D11_FILL_SOLID;
		rs.CullMode = D3D11_CULL_BACK;
		rs.FrontCounterClockwise = true;

		hr = device->CreateRasterizerState(&rs, &m_RasterizerState);
		if (FAILED(hr))
		{
			KBR_CORE_ERROR("Failed to create rasterizer state for pipeline!");
			KBR_CORE_ASSERT(false, "Failed to create rasterizer state for pipeline!");
		}

		D3D11_DEPTH_STENCIL_DESC ds{};
		ds.DepthEnable = spec.DepthTest != DepthTest::None;
		ds.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		ds.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;

		hr = device->CreateDepthStencilState(&ds, &m_DepthStencilState);
		if (FAILED(hr))
		{
			KBR_CORE_ERROR("Failed to create depth stencil state for pipeline!");
			KBR_CORE_ASSERT(false, "Failed to create depth stencil state for pipeline!");
		}

		D3D11_BLEND_DESC blend{};
		blend.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
		blend.RenderTarget[0].BlendEnable = FALSE;

		hr = device->CreateBlendState(&blend, &m_BlendState);
		if (FAILED(hr))
		{
			KBR_CORE_ERROR("Failed to create blend state for pipeline!");
			KBR_CORE_ASSERT(false, "Failed to create blend state for pipeline!");
		}
	}

	D3D11GraphicsPipeline::~D3D11GraphicsPipeline() = default;

	void D3D11GraphicsPipeline::Bind() const
	{
		const auto& context = D3D11Context::Get().GetImmediateContext();

		context->IASetInputLayout(m_InputLayout.Get());
		context->IASetPrimitiveTopology(m_PrimitiveTopology);

		m_Specification.Shader->Bind();

		context->RSSetState(m_RasterizerState.Get());
		context->OMSetDepthStencilState(m_DepthStencilState.Get(), 0);
		context->OMSetBlendState(m_BlendState.Get(), nullptr, 0xffffffff);
	}
}
