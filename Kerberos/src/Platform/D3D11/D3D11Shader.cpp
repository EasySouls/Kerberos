#include "kbrpch.h"
#include "D3D11Shader.h"
#include "D3D11Context.h"

#include "Kerberos/Utils/PlatformUtils.h"

#include <d3dcompiler.h>
#include <fstream>
#include <filesystem>
#include <ranges>

#include "D3D11Utils.h"

namespace Kerberos
{
	namespace Utils
	{
		static std::string GetFilenameWithoutExtension(const std::string& filepath)
		{
			const std::filesystem::path pathObj(filepath);
			return pathObj.stem().string();
		}
	}

	D3D11Shader::D3D11Shader(std::string filepath)
		: m_Name(std::move(filepath))
	{
		struct ShaderInfo
		{
			ShaderStage stage;
			std::string entryPoint;
			std::string profile;
		};

		// TODO: We should be able to configure it, or automatically detect it
		const std::array<ShaderInfo, 3> infos =
		{ {
			{ .stage = ShaderStage::Vertex, .entryPoint = "VS_Main", .profile ="vs_5_0" },
			{. stage = ShaderStage::Geometry, .entryPoint = "GS_Main", .profile = "gs_5_0" },
			{ .stage = ShaderStage::Fragment, .entryPoint = "PS_Main", .profile ="ps_5_0" }
		} };

		// TODO: This is a very nasty hack
		m_Name = Utils::GetFilenameWithoutExtension(m_Name);
		m_Name += ".hlsl";

		KBR_CORE_INFO("Compiling shader {0}", m_Name);

		uint32_t compileFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef KBR_DEBUG
		compileFlags |= D3DCOMPILE_SKIP_OPTIMIZATION | D3DCOMPILE_DEBUG;
#endif

		ComPtr<ID3DBlob> tempShaderBlob = nullptr;
		ComPtr<ID3DBlob> errorBlob = nullptr;

		std::unordered_map<ShaderStage, ComPtr<ID3DBlob>> shaderBlobs;

		for (const auto& [stage, entryPoint, profile] : infos)
		{
			// TODO: D3DCompileFromFile needs a relative filepath, like /assets/shaders/Shader.hlsl
			const auto relativePath = std::filesystem::path("assets") / "shaders" / std::filesystem::path(m_Name.data());
			const std::wstring filepathW = relativePath.wstring();

			const HRESULT hr = D3DCompileFromFile(
				filepathW.data(),
				nullptr,
				D3D_COMPILE_STANDARD_FILE_INCLUDE,
				entryPoint.c_str(),
				profile.c_str(),
				compileFlags,
				0,
				&tempShaderBlob,
				&errorBlob);

			if (FAILED(hr))
			{
				if (stage == ShaderStage::Geometry && hr == E_FAIL)
				{
					// Geometry shader is optional, so we can skip
					continue;
				}

				if (errorBlob != nullptr)
				{
					KBR_CORE_ERROR("Shader error log: {0}", static_cast<const char*>(errorBlob->GetBufferPointer()));
				}
				KBR_ASSERT(false, "Failed to compile shader from file");
			}

			shaderBlobs[stage] = tempShaderBlob;
		}

		for (const auto& blob : shaderBlobs | std::views::values)
		{
			const auto reflection = ReflectShader(blob);
			ReflectShaderInputs(reflection);
			ReflectShaderResources(reflection);
			ReflectConstantBuffers(reflection);
		}

		m_VertexShaderBlob = shaderBlobs[ShaderStage::Vertex];

		const auto& device = D3D11Context::Get().GetDevice();

		HRESULT hr = device->CreateVertexShader(
			shaderBlobs[ShaderStage::Vertex]->GetBufferPointer(),
			shaderBlobs[ShaderStage::Vertex]->GetBufferSize(),
			nullptr,
			&m_VertexShader);

		if (FAILED(hr))
		{
			KBR_CORE_ERROR("Failed to create vertex shader!");
			KBR_ASSERT(false, "Failed to create vertex shader");
		}

		hr = device->CreatePixelShader(
			shaderBlobs[ShaderStage::Fragment]->GetBufferPointer(),
			shaderBlobs[ShaderStage::Fragment]->GetBufferSize(),
			nullptr,
			&m_FragmentShader);

		if (FAILED(hr))
		{
			KBR_CORE_ERROR("Failed to create pixel shader!");
			KBR_ASSERT(false, "Failed to create pixel shader");
		}

		if (shaderBlobs.contains(ShaderStage::Geometry))
		{
			hr = device->CreateGeometryShader(
				shaderBlobs[ShaderStage::Geometry]->GetBufferPointer(),
				shaderBlobs[ShaderStage::Geometry]->GetBufferSize(),
				nullptr,
				&m_GeometryShader.emplace());

			if (FAILED(hr))
			{
				KBR_CORE_ERROR("Failed to create geometry shader!");
				KBR_ASSERT(false, "Failed to create geometry shader");
			}
		}

		KBR_CORE_INFO("Shader {0} compiled successfully", m_Name);
	}

	D3D11Shader::D3D11Shader(std::string name, const std::string& vertexSrc, const std::string& fragmentSrc,
		const std::string& geometrySrc)
		: m_Name(std::move(name))
	{
		throw std::logic_error("This method is deprecated, use the other constructor");
	}

	D3D11Shader::~D3D11Shader()
	{
		m_VertexShader.Reset();
		m_FragmentShader.Reset();
		if (m_GeometryShader.has_value())
		{
			m_GeometryShader->Reset();
		}
	}

	void D3D11Shader::Bind() const
	{
		const auto& context = D3D11Context::Get().GetImmediateContext();

		context->VSSetShader(m_VertexShader.Get(), nullptr, 0);
		context->PSSetShader(m_FragmentShader.Get(), nullptr, 0);
		if (m_GeometryShader.has_value())
		{
			context->GSSetShader(m_GeometryShader->Get(), nullptr, 0);
		}
	}

	void D3D11Shader::Unbind() const {}

	void D3D11Shader::SetDebugName(const std::string& name) const 
	{
		KBR_CORE_ASSERT(m_VertexShader, "Vertex shader is null!");
		KBR_CORE_ASSERT(m_FragmentShader, "Fragment shader is null!");

		D3D11Utils::SetDebugName(m_VertexShader.Get(), name + "_VertexShader");
		D3D11Utils::SetDebugName(m_FragmentShader.Get(), name + "_FragmentShader");
		if (m_GeometryShader.has_value())
		{
			D3D11Utils::SetDebugName(m_GeometryShader->Get(), name + "_GeometryShader");
		}
	}

	bool D3D11Shader::CompileShaderFromFile(const std::wstring& fileName, const std::string& entryPoint,
	                                        const std::string& profile, ComPtr<ID3DBlob>& shaderBlob)
	{
		constexpr UINT compileFlags = D3DCOMPILE_ENABLE_STRICTNESS;

		ComPtr<ID3DBlob> tempShaderBlob = nullptr;
		ComPtr<ID3DBlob> errorBlob = nullptr;

		const HRESULT hr = D3DCompileFromFile(
			fileName.data(),
			nullptr,
			D3D_COMPILE_STANDARD_FILE_INCLUDE,
			entryPoint.c_str(),
			profile.c_str(),
			compileFlags,
			0,
			&tempShaderBlob,
			&errorBlob);

		if (FAILED(hr))
		{
			if (errorBlob != nullptr)
			{
				KBR_CORE_ERROR("Shader error log: {0}", static_cast<const char*>(errorBlob->GetBufferPointer()));
			}
			KBR_ASSERT(false, "Failed to compile shader from file");
			return false;
		}

		shaderBlob = std::move(tempShaderBlob);
		return true;
	}

	bool D3D11Shader::CompileShaderFromSource(const std::string& source, const std::string& entryPoint, const std::string& profile, ComPtr<ID3DBlob>& shaderBlob)
	{
		constexpr UINT compileFlags = D3DCOMPILE_ENABLE_STRICTNESS;

		ComPtr<ID3DBlob> tempShaderBlob = nullptr;
		ComPtr<ID3DBlob> errorBlob = nullptr;

		const HRESULT hr = D3DCompile(
			source.data(),
			source.size(),
			nullptr,
			nullptr,
			D3D_COMPILE_STANDARD_FILE_INCLUDE,
			entryPoint.c_str(),
			profile.c_str(),
			compileFlags,
			0,
			&tempShaderBlob,
			&errorBlob);

		if (FAILED(hr))
		{
			KBR_CORE_ERROR("Failed to compile shader from source: {0}", source);
			if (errorBlob != nullptr)
			{
				KBR_CORE_ERROR("{0}", static_cast<const char*>(errorBlob->GetBufferPointer()));
			}
			KBR_ASSERT(false, "Failed to compile shader from source");
			return false;
		}

		shaderBlob = std::move(tempShaderBlob);
		return true;
	}

	std::string D3D11Shader::ReadFile(const std::string& filepath)
	{
		std::ifstream in(filepath, std::ios::in | std::ios::binary);

		if (!in)
		{
			KBR_CORE_ERROR("Could not open file '{0}'", filepath);
			KBR_CORE_ASSERT(false, "File not found!");
			return "";
		}

		/// Check the size of the file and resize the string
		std::string result;
		in.seekg(0, std::ios::end);
		result.resize(in.tellg());

		/// Read the shaders into the string
		in.seekg(0, std::ios::beg);
		in.read(result.data(), static_cast<std::streamsize>(result.size()));

		in.close();

		return result;
	}

	ComPtr<ID3D11ShaderReflection> D3D11Shader::ReflectShader(const ComPtr<ID3DBlob>& shaderBlob)
	{
		ComPtr<ID3D11ShaderReflection> reflection;

		const HRESULT hr = D3DReflect(
			shaderBlob->GetBufferPointer(),
			shaderBlob->GetBufferSize(),
			IID_ID3D11ShaderReflection,
			reinterpret_cast<void**>(reflection.GetAddressOf())
		);

		if (FAILED(hr))
		{
			KBR_CORE_ERROR("Failed to reflect shader!");
			return nullptr;
		}

		D3D11_SHADER_DESC shaderDesc;
		reflection->GetDesc(&shaderDesc);

		KBR_CORE_INFO("\tConstant buffer count: {}", shaderDesc.ConstantBuffers);
		KBR_CORE_INFO("\tBound resources count: {}", shaderDesc.BoundResources);
		KBR_CORE_INFO("\tInstruction count: {}", shaderDesc.InstructionCount);

		return reflection;
	}

	void D3D11Shader::ReflectShaderInputs(const ComPtr<ID3D11ShaderReflection>& reflection)
	{
		D3D11_SHADER_DESC shaderDesc;
		reflection->GetDesc(&shaderDesc);

		KBR_CORE_INFO("Shader Inputs:");

		for (uint32_t i = 0; i < shaderDesc.InputParameters; ++i)
		{
			D3D11_SIGNATURE_PARAMETER_DESC paramDesc;
			reflection->GetInputParameterDesc(i, &paramDesc);

			KBR_CORE_INFO("\tSemantic Name: {}, Semantic Index: {}, Register: {}, System Value Type: {}, Component Type: {}, Mask: {}",
				paramDesc.SemanticName,
				paramDesc.SemanticIndex,
				paramDesc.Register,
				static_cast<uint32_t>(paramDesc.SystemValueType),
				static_cast<uint32_t>(paramDesc.ComponentType),
				paramDesc.Mask);
		}
	}

	void D3D11Shader::ReflectShaderResources(const ComPtr<ID3D11ShaderReflection>& reflection)
	{
		D3D11_SHADER_DESC shaderDesc;
		reflection->GetDesc(&shaderDesc);

		KBR_CORE_INFO("Shader Resources:");

		for (uint32_t i = 0; i < shaderDesc.BoundResources; ++i)
		{
			D3D11_SHADER_INPUT_BIND_DESC bindDesc;
			reflection->GetResourceBindingDesc(i, &bindDesc);

			KBR_CORE_INFO("\tName: {}, Idx: {}, Type: {}, Bind Point: {}, Bind Count: {}, Flags: {}",
				bindDesc.Name,
				i,
				static_cast<uint32_t>(bindDesc.Type),
				bindDesc.BindPoint,
				bindDesc.BindCount,
				static_cast<uint32_t>(bindDesc.uFlags));
		}
	}

	void D3D11Shader::ReflectConstantBuffers(const ComPtr<ID3D11ShaderReflection>& reflection)
	{
		D3D11_SHADER_DESC shaderDesc;
		reflection->GetDesc(&shaderDesc);

		KBR_CORE_INFO("Constant Buffers:");

		for (uint32_t i = 0; i < shaderDesc.ConstantBuffers; ++i)
		{
			ID3D11ShaderReflectionConstantBuffer* constantBuffer = reflection->GetConstantBufferByIndex(i);
			D3D11_SHADER_BUFFER_DESC bufferDesc;
			constantBuffer->GetDesc(&bufferDesc);

			KBR_CORE_INFO("\tName: {}, Variables: {}, Size: {}, Type: {}",
				bufferDesc.Name,
				bufferDesc.Variables,
				bufferDesc.Size,
				static_cast<uint32_t>(bufferDesc.Type));

			for (uint32_t j = 0; j < bufferDesc.Variables; ++j)
			{
				ID3D11ShaderReflectionVariable* variable = constantBuffer->GetVariableByIndex(j);
				D3D11_SHADER_VARIABLE_DESC varDesc;
				variable->GetDesc(&varDesc);

				KBR_CORE_INFO("\t\tVariable Name: {}, Start Offset: {}, Size: {}",
					varDesc.Name,
					varDesc.StartOffset,
					varDesc.Size);
			}
		}
	}
}
