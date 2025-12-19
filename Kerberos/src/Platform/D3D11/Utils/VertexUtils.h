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

	constexpr VertexFormat GetFormatFromComponentTypeAndMask(D3D_REGISTER_COMPONENT_TYPE componentType, uint8_t mask)
	{
		const uint32_t componentCount =
			(mask & 1 ? 1 : 0) +
			(mask & 2 ? 1 : 0) +
			(mask & 4 ? 1 : 0) +
			(mask & 8 ? 1 : 0);

		switch (componentType)
		{
		case D3D_REGISTER_COMPONENT_FLOAT32:
			{
				if (componentCount == 1)
					return VertexFormat::Float1;
				if (componentCount == 2)
					return VertexFormat::Float2;
				if (componentCount == 3)
					return VertexFormat::Float3;
				if (componentCount == 4)
					return VertexFormat::Float4;

				break;
			}
		case D3D_REGISTER_COMPONENT_SINT32:
			{
				if (componentCount == 1)
					return VertexFormat::Int1;
				if (componentCount == 2)
					return VertexFormat::Int2;
				if (componentCount == 3)
					return VertexFormat::Int3;
				if (componentCount == 4)
					return VertexFormat::Int4;

				break;
			}
		case D3D_REGISTER_COMPONENT_UINT32:
			{
				if (componentCount == 1)
					return VertexFormat::UInt1;
				if (componentCount == 2)
					return VertexFormat::UInt2;
				if (componentCount == 3)
					return VertexFormat::UInt3;
				if (componentCount == 4)
					return VertexFormat::UInt4;

				break;
			}
		case D3D_REGISTER_COMPONENT_UNKNOWN:
		case D3D_REGISTER_COMPONENT_UINT16:
		case D3D_REGISTER_COMPONENT_SINT16:
		case D3D_REGISTER_COMPONENT_FLOAT16:
		case D3D_REGISTER_COMPONENT_UINT64:
		case D3D_REGISTER_COMPONENT_SINT64:
		case D3D_REGISTER_COMPONENT_FLOAT64:
			break;
		}
		KBR_CORE_ASSERT(false, "Not supported vertex input!");
		return VertexFormat::Float4;
	}

	constexpr ShaderResourceType MapType(const D3D_SHADER_INPUT_TYPE t)
	{
		switch (t)
		{
		case D3D_SIT_CBUFFER:   return ShaderResourceType::ConstantBuffer;
		case D3D_SIT_TEXTURE:   return ShaderResourceType::Texture;
		case D3D_SIT_SAMPLER:   return ShaderResourceType::Sampler;
		case D3D_SIT_STRUCTURED:
		case D3D_SIT_BYTEADDRESS:
		case D3D_SIT_UAV_RWTYPED:
			return ShaderResourceType::UnorderedAccessView;
		case D3D_SIT_TBUFFER:
		case D3D_SIT_UAV_RWSTRUCTURED:
		case D3D_SIT_UAV_RWBYTEADDRESS:
		case D3D_SIT_UAV_APPEND_STRUCTURED:
		case D3D_SIT_UAV_CONSUME_STRUCTURED:
		case D3D_SIT_UAV_RWSTRUCTURED_WITH_COUNTER:
		case D3D_SIT_RTACCELERATIONSTRUCTURE:
		case D3D_SIT_UAV_FEEDBACKTEXTURE:
			break;
		}

		KBR_CORE_ASSERT(false, "Unknown shader resource type!");
		return ShaderResourceType::ConstantBuffer;
	}
}