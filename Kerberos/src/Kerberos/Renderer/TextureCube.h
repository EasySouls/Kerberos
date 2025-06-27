#pragma once
#include "Texture.h"

#include <filesystem>

namespace Kerberos
{
	struct FaceData
	{
		TextureSpecification Specification;
		Buffer Buffer;
	};
	struct CubemapData
	{
		std::string Name;
		std::array<FaceData, 6> Faces;
		bool IsSRGB = false;
	};

	class TextureCube : public Texture
	{
	public:
		~TextureCube() override = default;

		virtual const std::string& GetName() const = 0;

		AssetType GetType() override { return AssetType::TextureCube; }

		static Ref<TextureCube> Create(const std::string& name, const std::vector<std::string>& faces, bool generateMipmaps = true, bool srgb = false);
		static Ref<TextureCube> Create(const CubemapData& data);
	};
}
