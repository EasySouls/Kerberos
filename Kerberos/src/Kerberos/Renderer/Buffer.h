#pragma once
#include "Kerberos/Core.h"
#include "Kerberos/Log.h"

namespace Kerberos
{
	enum class ShaderDataType : uint8_t
	{
		None = 0, Float, Float2, Float3, Float4, Mat3, Mat4, Int, Int2, Int3, Int4, Bool
	};

	static uint32_t ShaderDataTypeSize(ShaderDataType type)
	{
		switch (type)
		{
			case ShaderDataType::None:     return 0;
			case ShaderDataType::Float:    return 4;
			case ShaderDataType::Float2:   return 4 * 2;
			case ShaderDataType::Float3:   return 4 * 3;
			case ShaderDataType::Float4:   return 4 * 4;
			case ShaderDataType::Mat3:     return 4 * 3 * 3;
			case ShaderDataType::Mat4:     return 4 * 4 * 4;
			case ShaderDataType::Int:      return 4;
			case ShaderDataType::Int2:     return 4 * 2;
			case ShaderDataType::Int3:     return 4 * 3;
			case ShaderDataType::Int4:     return 4 * 4;
			case ShaderDataType::Bool:     return 1;
		}

		KBR_ASSERT(false, "Unknown ShaderDataType!");
		return 0;
	}

	struct BufferElement
	{
		std::string Name;
		ShaderDataType Type;
		uint32_t Size;
		uint32_t Offset;
		bool Normalized;

		BufferElement() = default;

		BufferElement(const ShaderDataType type, std::string name, const bool normalized = false)
			: Name(std::move(name)), Type(type), Size(ShaderDataTypeSize(type)), Offset(0), Normalized(normalized)
		{}

		uint32_t GetComponentCount() const
		{
			switch (Type)
			{
				case ShaderDataType::None:     return 0;
				case ShaderDataType::Float:    return 1;
				case ShaderDataType::Float2:   return 2;
				case ShaderDataType::Float3:   return 3;
				case ShaderDataType::Float4:   return 4;
				case ShaderDataType::Mat3:     return 3 * 3;
				case ShaderDataType::Mat4:     return 4 * 4;
				case ShaderDataType::Int:      return 1;
				case ShaderDataType::Int2:     return 2;
				case ShaderDataType::Int3:     return 3;
				case ShaderDataType::Int4:     return 4;
				case ShaderDataType::Bool:     return 1;
			}

			KBR_ASSERT(false, "Unknown ShaderDataType!");
			return 0;
		}
	};

	class BufferLayout
	{
	public:
		BufferLayout() = default;

		BufferLayout(const std::initializer_list<BufferElement>& elements)
			: m_Elements(elements), m_Stride(0)
		{
			CalculateOffsetsAndStride();
		}

		const std::vector<BufferElement>& GetElements() const { return m_Elements; }
		uint32_t GetStride() const { return m_Stride; }

		std::vector<BufferElement>::iterator begin() { return m_Elements.begin(); }
		std::vector<BufferElement>::iterator end() { return m_Elements.end(); }
		std::vector<BufferElement>::const_iterator begin() const { return m_Elements.begin(); }
		std::vector<BufferElement>::const_iterator end() const { return m_Elements.end(); }
	private:
		void CalculateOffsetsAndStride()
		{
			uint32_t offset = 0;
			m_Stride = 0;
			for (auto& elem : m_Elements)
			{
				elem.Offset = offset;
				offset += elem.Size;
				m_Stride += elem.Size;
			}
		}

	private:
		std::vector<BufferElement> m_Elements;
		uint32_t m_Stride;
	};

	class VertexBuffer
	{
	public:
		virtual ~VertexBuffer() = default;

		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;

		virtual void SetData(const void* data, uint32_t size) = 0;

		virtual void SetLayout(const BufferLayout& layout) = 0;
		virtual const BufferLayout& GetLayout() const = 0;
		virtual uint32_t GetCount() const = 0;

		template<typename T>
		T& As()
		{
			return *static_cast<T*>(this);
		}

		static Ref<VertexBuffer> Create(const float* vertices, uint32_t size);
		static Ref<VertexBuffer> Create(uint32_t size);
	};

	class IndexBuffer
	{
	public:
		virtual ~IndexBuffer() = default;

		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;

		virtual uint32_t GetCount() const = 0;

        template<typename T>
		T& As()
		{
			return *static_cast<T*>(this);
		}

		static Ref<IndexBuffer> Create(const uint32_t* indices, uint32_t count);
	};
}
