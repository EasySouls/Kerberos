#pragma once
#include "Kerberos/Assets/AssetMetadata.h"
#include "Kerberos/Renderer/Texture.h"

namespace Kerberos
{
	class TextureImporter
	{
	public:
		static Ref<Texture2D> ImportTexture(AssetHandle handle, const AssetMetadata& metadata);
		static Ref<Texture2D> ImportTexture(const std::filesystem::path& filepath);

		static std::pair<TextureSpecification, Buffer> LoadTextureData(const std::filesystem::path& filepath, bool flip);
	};
}
