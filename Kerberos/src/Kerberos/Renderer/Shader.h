#pragma once

#include "Kerberos/Core.h"
#include "Kerberos/Renderer/Material.h"

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <glm/glm.hpp>

namespace Kerberos
{
	enum class ShaderStage : std::uint8_t
	{
		Vertex,
		Geometry,
		Fragment
	};

	enum class ShaderResourceType : std::uint8_t
	{
		Texture,
		Sampler,
		ConstantBuffer,
		UnorderedAccessView,
		StructuredBuffer
	};

	enum class VertexFormat : std::uint8_t
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

	inline void HashCombine(size_t& seed, size_t value)
	{
		seed ^= value + 0x9e3779b9 + (seed << 6) + (seed >> 2);
	}

	struct ShaderVertexInput
	{
		std::string		semantic;
		uint32_t		semanticIndex;
		VertexFormat	format;
		uint32_t		location;
	};

	struct ShaderResourceBinding
	{
		std::string			name;
		ShaderResourceType	type;
		uint32_t			bindPoint;
		uint32_t			bindCount;

		bool operator==(const ShaderResourceBinding&) const = default;
	};

	struct ShaderUniform
	{
		std::string name;
		uint32_t	offset;
		uint32_t	size;

		bool operator==(const ShaderUniform&) const = default;
	};

	struct ShaderUniformHash
	{
		size_t operator()(const ShaderUniform& s) const noexcept
		{
			size_t seed = std::hash<std::string>{}(s.name);
			HashCombine(seed, std::hash<uint32_t>{}(s.offset));
			HashCombine(seed, std::hash<uint32_t>{}(s.size));
			return seed;
		}
	};

	struct ShaderResourceBindingHash
	{
		size_t operator()(const ShaderResourceBinding& s) const noexcept
		{
			size_t seed = std::hash<std::string>{}(s.name);
			HashCombine(seed, std::hash<int>{}(static_cast<int>(s.type)));
			HashCombine(seed, std::hash<uint32_t>{}(s.bindPoint));
			HashCombine(seed, std::hash<uint32_t>{}(s.bindCount));
			return seed;
		}
	};

	struct ShaderConstantBuffer
	{
		std::string							name;
		uint32_t							size;
		uint32_t							slot;
		std::unordered_set<ShaderUniform, ShaderUniformHash>	uniforms;

		bool operator==(const ShaderConstantBuffer&) const = default;
	};

	struct ShaderConstantBufferHash
	{
		size_t operator()(const ShaderConstantBuffer& s) const noexcept
		{
			size_t seed = std::hash<std::string>{}(s.name);
			HashCombine(seed, std::hash<uint32_t>{}(s.size));
			HashCombine(seed, std::hash<uint32_t>{}(s.slot));
			for (const auto& u : s.uniforms)
			{
				HashCombine(seed, ShaderUniformHash{}(u));
			}
			return seed;
		}
	};

	struct ShaderReflectionData
	{
		std::vector<ShaderVertexInput>				vertexInputs;
		std::unordered_set<ShaderResourceBinding, ShaderResourceBindingHash>	resources;
		std::unordered_set<ShaderConstantBuffer, ShaderConstantBufferHash>	constantBuffers;
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