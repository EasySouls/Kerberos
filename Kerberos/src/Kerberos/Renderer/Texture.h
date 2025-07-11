#pragma once

#include "Kerberos/Core.h"
#include "Kerberos/Core/Buffer.h"
#include "Kerberos/Assets/Asset.h"
#include <string>

namespace Kerberos
{
	enum class ImageFormat
	{
		None = 0,
		R8,
		RGB8,
		RGBA8,
		RGBA32F
	};

	struct TextureSpecification
	{
		uint32_t Width = 1;
		uint32_t Height = 1;
		ImageFormat Format = ImageFormat::RGBA8;
		bool GenerateMips = false;
	};

	class Texture : public Asset
	{
	public:
		~Texture() override = default;

		virtual uint32_t GetWidth() const = 0;
		virtual uint32_t GetHeight() const = 0;

		virtual uint64_t GetRendererID() const = 0;
		virtual const TextureSpecification& GetSpecification() const = 0;
		
		virtual void Bind(uint32_t slot = 0) const = 0;

		virtual void SetData(void* data, uint32_t size) = 0;

		virtual bool operator==(const Texture& other) const = 0;

		virtual void SetDebugName(const std::string& name) const = 0;

		template<typename T>
		T& As()
		{
			return *static_cast<T*>(this);
		}
	};

	class Texture2D : public Texture
	{
	public:
		static Ref<Texture2D> Create(const TextureSpecification& spec, Buffer data = Buffer());

		AssetType GetType() override { return AssetType::Texture2D; }
	};
}
