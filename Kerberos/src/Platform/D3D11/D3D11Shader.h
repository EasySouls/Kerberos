#pragma once

#include "Kerberos/Renderer/Shader.h"

#include <d3d11.h>
#include <wrl.h>

namespace Kerberos
{
	using Microsoft::WRL::ComPtr;

	class D3D11Shader : public Shader
	{
	public:
		explicit D3D11Shader(const std::string& filepath);
		D3D11Shader(std::string name, const std::string& vertexSrc, const std::string& fragmentSrc, const std::string& geometrySrc = "");
		~D3D11Shader() override;

		void Bind() const override;
		void Unbind() const override;

	private:
		bool CompileShader(const std::wstring& fileName, const std::string& entryPoint, const std::string& profile, ComPtr<ID3DBlob>& shaderBlob) const;

		[[nodiscard]] 
		ComPtr<ID3D11VertexShader> CreateVertexShader(const std::wstring& fileName, ComPtr<ID3DBlob>& vertexShaderBlob) const;

		[[nodiscard]] 
		ComPtr<ID3D11PixelShader> CreatePixelShader(std::wstring& fileName) const;

	};
}

