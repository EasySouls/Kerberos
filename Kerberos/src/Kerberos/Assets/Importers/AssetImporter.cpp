#include "kbrpch.h"
#include "AssetImporter.h"

#include "CubemapImporter.h"
#include "TextureImporter.h"
#include "Kerberos/Renderer/Texture.h"

namespace Kerberos
{
	Ref<Asset> AssetImporter::ImportAsset(const AssetHandle handle, const AssetMetadata& metadata) 
	{
		switch (metadata.Type)
		{
		case AssetType::Texture2D:
			return TextureImporter::ImportTexture(handle, metadata);
		case AssetType::TextureCube:
			return CubemapImporter::ImportCubemap(handle, metadata);
			break;
		case AssetType::Material:
		case AssetType::Mesh:
		case AssetType::Scene:
			break;
		}

		KBR_CORE_ASSERT(false, "Unsupported asset type by AssetImporter!");
		return nullptr;
	}
}
