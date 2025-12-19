#pragma once

#include "Kerberos/Renderer/Shader.h"
#include "Platform/D3D11/D3D11Utils.h"

#include <d3d11.h>
#include <vector>
#include <ranges>

namespace Kerberos::VertexUtils
{
	constexpr DXGI_FORMAT VertexFormatToDXGIFormat(const VertexFormat format)
	{
		switch (format)
		{
		case VertexFormat::Float1: return DXGI_FORMAT_R32_FLOAT;
		case VertexFormat::Float2: return DXGI_FORMAT_R32G32_FLOAT;
		case VertexFormat::Float3: return DXGI_FORMAT_R32G32B32_FLOAT;
		case VertexFormat::Float4: return DXGI_FORMAT_R32G32B32A32_FLOAT;
		case VertexFormat::Int1:   return DXGI_FORMAT_R32_SINT;
		case VertexFormat::Int2:   return DXGI_FORMAT_R32G32_SINT;
		case VertexFormat::Int3:   return DXGI_FORMAT_R32G32B32_SINT;
		case VertexFormat::Int4:   return DXGI_FORMAT_R32G32B32A32_SINT;
		case VertexFormat::UInt1:  return DXGI_FORMAT_R32_UINT;
		case VertexFormat::UInt2:  return DXGI_FORMAT_R32G32_UINT;
		case VertexFormat::UInt3:  return DXGI_FORMAT_R32G32B32_UINT;
		case VertexFormat::UInt4:  return DXGI_FORMAT_R32G32B32A32_UINT;
		}

		KBR_CORE_ASSERT(false, "Unknown VertexFormat!");
		return DXGI_FORMAT_UNKNOWN;
	}


	constexpr std::vector<D3D11_INPUT_ELEMENT_DESC> CreateInputLayoutFromVertexInputs(const std::vector<ShaderVertexInput>& inputs)
	{
		std::vector<D3D11_INPUT_ELEMENT_DESC> elements = 
			inputs 
			| std::views::transform([](const ShaderVertexInput& input) -> D3D11_INPUT_ELEMENT_DESC
			{
				D3D11_INPUT_ELEMENT_DESC elementDesc;
				elementDesc.SemanticName = input.semantic.c_str();
				elementDesc.SemanticIndex = input.semanticIndex;
				elementDesc.Format = VertexFormatToDXGIFormat(input.format);
				elementDesc.InputSlot = 0;
				elementDesc.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
				elementDesc.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
				elementDesc.InstanceDataStepRate = 0;
				return elementDesc;
			})
			| std::ranges::to<std::vector>();

		return elements;
	}
}