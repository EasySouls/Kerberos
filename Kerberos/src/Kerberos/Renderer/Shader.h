#pragma once

#include "Kerberos/Core.h"
#include "Kerberos/Renderer/Material.h"

#include <string>
#include <unordered_map>
#include <glm/glm.hpp>

namespace Kerberos
{
	enum class ShaderStage
	{
		Vertex,
		Geometry,
		Fragment
	};

	enum class ShaderResourceType
	{
		Texture,
		Sampler,
		ConstantBuffer,
		UnorderedAccessView,
		StructuredBuffer
	};

	enum class VertexFormat
	{
		Float1,
		Float2,
		Float3,
		Float4,
		Int1,
		Int2,
		Int3,
		Int4,
		UInt1,
		UInt2,
		UInt3,
		UInt4,
	};

	struct ShaderVertexInput
	{
		std::string semantic;   // POSITION, NORMAL, TEXCOORD
		uint32_t    semanticIndex;
		VertexFormat format;    // Float3, Float2, etc.
		uint32_t    location;   // paramDesc.Register
	};

	struct ShaderResourceBinding
	{
		std::string name;
		ShaderResourceType type; // Texture, Sampler, CB, UAV, etc.
		uint32_t bindPoint;
		uint32_t bindCount;
	};

	struct ShaderUniform
	{
		std::string name;
		uint32_t offset;
		uint32_t size;
	};

	struct ShaderConstantBuffer
	{
		std::string name;
		uint32_t size;
		uint32_t slot;
		std::vector<ShaderUniform> uniforms;
	};

	struct ShaderReflectionData
	{
		std::vector<ShaderVertexInput>    vertexInputs;
		std::vector<ShaderResourceBinding> resources;
		std::vector<ShaderConstantBuffer> constantBuffers;
	};

	class Shader
	{
	public:
		virtual ~Shader() = default;

		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;

		virtual void SetInt(const std::string& name, int value) = 0;
		virtual void SetIntArray(const std::string& name, int* values, uint32_t count) = 0;
		virtual void SetFloat(const std::string& name, float value) = 0;
		virtual void SetFloat3(const std::string& name, const glm::vec3& value) = 0;
		virtual void SetFloat4(const std::string& name, const glm::vec4& value) = 0;
		virtual void SetMat4(const std::string& name, const glm::mat4& value) = 0;

		virtual void SetMaterial(const std::string& name, const Ref<Material>& material) = 0;

		virtual const std::string& GetName() const = 0;
		virtual const ShaderReflectionData& GetReflectionData() const = 0;

		virtual void SetDebugName(const std::string& name) const = 0;

		template<typename T>
		T& As()
		{
			return *static_cast<T*>(this);
		}

		static Ref<Shader> Create(const std::string& filepath);
		static Ref<Shader> Create(const std::string& name, const std::string& vertexSrc, const std::string& fragmentSrc);
	};

	class ShaderLibrary
	{
	public:
		void Add(const Ref<Shader>& shader);
		void Add(const std::string& name, const Ref<Shader>& shader);

		Ref<Shader> Load(const std::string& filepath);
		Ref<Shader> Load(const std::string& name, const std::string& filepath);

		Ref<Shader> Get(const std::string& name);

		bool Exists(const std::string& name) const;
	private:
		std::unordered_map<std::string, Ref<Shader>> m_Shaders;
	};
}
