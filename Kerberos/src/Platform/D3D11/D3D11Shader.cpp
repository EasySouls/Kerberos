#include "kbrpch.h"
#include "D3D11Shader.h"
#include "D3D11Context.h"

#include <d3dcompiler.h>
#include <fstream>
#include <filesystem>

namespace Kerberos
{
	D3D11Shader::D3D11Shader(std::string filepath)
		: m_Name(std::move(filepath))
	{}

	D3D11Shader::D3D11Shader(std::string name, const std::string& vertexSrc, const std::string& fragmentSrc,
		const std::string& geometrySrc)
		: m_Name(std::move(name))
	{}

	D3D11Shader::~D3D11Shader() {}
	void D3D11Shader::Bind() const {}
	void D3D11Shader::Unbind() const {}

	void D3D11Shader::SetDebugName(const std::string& name) const 
	{
		const HRESULT res = D3D11Context::Get().GetDevice()->SetPrivateData(WKPDID_D3DDebugObjectName, static_cast<UINT>(name.size()), name.data());
		if (FAILED(res))
		{
			KBR_CORE_WARN("Failed to set shader debug name: {0}", name);
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

	ComPtr<ID3D11VertexShader> D3D11Shader::CreateVertexShader(const std::wstring& fileName,
		ComPtr<ID3DBlob>& vertexShaderBlob)
	{
		const std::string vertexShaderSource = ReadFile(std::string(fileName.begin(), fileName.end()));

		if (!CompileShaderFromSource(vertexShaderSource, "Main", "vs_5_0", vertexShaderBlob))
		{
			KBR_CORE_ERROR("Failed to compile vertex shader");
			return nullptr;
		}

		ComPtr<ID3D11VertexShader> vertexShader;
		const HRESULT hr = D3D11Context::Get().GetDevice()->CreateVertexShader(
			vertexShaderBlob->GetBufferPointer(),
			vertexShaderBlob->GetBufferSize(),
			nullptr,
			&vertexShader);

		if (FAILED(hr))
		{
			KBR_CORE_ERROR("Failed to create vertex shader");
			return nullptr;
		}

		return vertexShader;
	}

	ComPtr<ID3D11PixelShader> D3D11Shader::CreatePixelShader(const std::wstring& fileName) 
	{
		ComPtr<ID3DBlob> pixelShaderBlob = nullptr;
		const std::string pixelShaderSource = ReadFile(std::string(fileName.begin(), fileName.end()));

		if (!CompileShaderFromSource(pixelShaderSource, "Main", "ps_5_0", pixelShaderBlob))
		{
			KBR_CORE_ERROR("Failed to compile pixel shader");
			return nullptr;
		}

		ComPtr<ID3D11PixelShader> pixelShader;
		const HRESULT hr = D3D11Context::Get().GetDevice()->CreatePixelShader(
			pixelShaderBlob->GetBufferPointer(),
			pixelShaderBlob->GetBufferSize(),
			nullptr,
			&pixelShader);

		if (FAILED(hr))
		{
			KBR_CORE_ERROR("Failed to create pixel shader");
			return nullptr;
		}

		return pixelShader;
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
}
