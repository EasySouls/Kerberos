#include "kbrpch.h"
#include "D3D11Shader.h"

#include <d3dcompiler.h>

namespace Kerberos
{
	D3D11Shader::D3D11Shader(const std::string& filepath) {}

	D3D11Shader::D3D11Shader(std::string name, const std::string& vertexSrc, const std::string& fragmentSrc,
		const std::string& geometrySrc) {}

	D3D11Shader::~D3D11Shader() {}
	void D3D11Shader::Bind() const {}
	void D3D11Shader::Unbind() const {}

	bool D3D11Shader::CompileShader(const std::wstring& fileName, const std::string& entryPoint,
		const std::string& profile, ComPtr<ID3DBlob>& shaderBlob) const 
	{
		constexpr UINT compileFlags = D3DCOMPILE_ENABLE_STRICTNESS;

		ComPtr<ID3DBlob> tempShaderBlob;
		ComPtr<ID3DBlob> errorBlob;

		HRESULT hr = D3DCompileFromFile(fileName.data(),
			nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, entryPoint.c_str(), profile.c_str(),
			compileFlags, 0, &tempShaderBlob, &errorBlob);
	}

	ComPtr<ID3D11VertexShader> D3D11Shader::CreateVertexShader(const std::wstring& fileName,
	                                                           ComPtr<ID3DBlob>& vertexShaderBlob) const {}

	ComPtr<ID3D11PixelShader> D3D11Shader::CreatePixelShader(std::wstring& fileName) const {}
}
