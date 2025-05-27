#pragma once

#include "Kerberos/Renderer/Shader.h"

#include <d3d11.h>
#include <wrl.h>

namespace Kerberos
{
	class D3D11Context;

	using Microsoft::WRL::ComPtr;

	class D3D11Shader final : public Shader
	{
	public:
		explicit D3D11Shader(const std::string& filepath);
		D3D11Shader(std::string name, const std::string& vertexSrc, const std::string& fragmentSrc, const std::string& geometrySrc = "");
		~D3D11Shader() override;

		void Bind() const override;
		void Unbind() const override;

		void SetInt(const std::string& name, int value) override {}
		void SetIntArray(const std::string& name, int* values, uint32_t count) override {}
		void SetFloat(const std::string& name, float value) override {}
		void SetFloat3(const std::string& name, const glm::vec3& value) override {}
		void SetFloat4(const std::string& name, const glm::vec4& value) override {}
		void SetMat4(const std::string& name, const glm::mat4& value) override {}

		void SetMaterial(const std::string& name, const Ref<Material>& material) override {}

		const std::string& GetName() const override { return "D3D11 Shader"; }

	private:
		/*
		* Compiles the shader from the given file.
		* @param fileName The path to the shader file.
		* @param entryPoint The entry point function name in the shader.
		* @param profile The shader profile (e.g., "vs_5_0" for vertex shader, "ps_5_0" for pixel shader).
		* @param shaderBlob The output blob containing the compiled shader code.
		* @return True if compilation was successful, false otherwise.
		*/
		static bool CompileShaderFromFile(const std::wstring& fileName, const std::string& entryPoint, const std::string& profile, ComPtr<ID3DBlob>& shaderBlob);

		/*
		* Compiles the shader from the given source code.
		* @param source The shader source code as a string.
		* @param entryPoint The entry point function name in the shader.
		* @param profile The shader profile (e.g., "vs_5_0" for vertex shader, "ps_5_0" for pixel shader).
		* @param shaderBlob The output blob containing the compiled shader code.
		* @return True if compilation was successful, false otherwise.
		*/
		static bool CompileShaderFromSource(const std::string& source, const std::string& entryPoint, const std::string& profile, ComPtr<ID3DBlob>& shaderBlob);

		[[nodiscard]] 
		static ComPtr<ID3D11VertexShader> CreateVertexShader(const std::wstring& fileName, ComPtr<ID3DBlob>& vertexShaderBlob);

		[[nodiscard]] 
		static ComPtr<ID3D11PixelShader> CreatePixelShader(const std::wstring& fileName);

	private:
		friend class D3D11Context;
	};
}

