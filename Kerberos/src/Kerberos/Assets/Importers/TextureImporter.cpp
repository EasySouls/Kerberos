#include "kbrpch.h"
#include "TextureImporter.h"

#include <stb_image.h>

namespace Kerberos
{
	Ref<Texture2D> TextureImporter::ImportTexture(AssetHandle handle, const AssetMetadata& metadata) 
	{
		return ImportTexture(metadata.Filepath);
	}

	Ref<Texture2D> TextureImporter::ImportTexture(const std::filesystem::path& filepath)
	{
		KBR_PROFILE_FUNCTION();

		const auto [spec, data] = LoadTextureData(filepath);

		auto texture = Texture2D::Create(spec, data);

		stbi_image_free(data.Data);

		return texture;
	}

	std::pair<TextureSpecification, Buffer> TextureImporter::LoadTextureData(const std::filesystem::path& filepath) 
	{
		int width, height, channels;
		//stbi_set_flip_vertically_on_load(1);
		Buffer data;

		{
			KBR_PROFILE_SCOPE("TextureImporter::ImportTexture - stbi_load");
			data.Data = stbi_load(filepath.string().c_str(), &width, &height, &channels, 0);
		}

		if (data.Data == nullptr)
		{
			KBR_CORE_ERROR("TextureImporter::ImportTexture - failed to load texture from filepath: {}", filepath.string());
			return std::make_pair(TextureSpecification{}, Buffer{});
		}

		data.Size = static_cast<uint64_t>(width) * height * channels;

		TextureSpecification spec;
		spec.Width = width;
		spec.Height = height;
		switch (channels)
		{
		case 3:
			spec.Format = ImageFormat::RGB8;
			break;
		case 4:
			spec.Format = ImageFormat::RGBA8;
			break;
		default:
			KBR_CORE_ASSERT(false, "TextureImporter::ImportTexture - unsupported number of image channels: {}", channels);
			break;
		}

		return std::make_pair(spec, data);
	}
}
