#include "kbrpch.h"
#include "AssetImporter.h"

#include "CubemapImporter.h"
#include "TextureImporter.h"
#include "MeshImporter.h"
#include "Kerberos/Renderer/Texture.h"

namespace Kerberos
{
	void AssetImporter::Init()
	{
		m_ThreadPool.Enqueue([]()
		{
			KBR_CORE_INFO("AssetImporter thread pool initialized with 4 threads.");
			});
	}

	Ref<Asset> AssetImporter::ImportAsset(const AssetHandle handle, const AssetMetadata& metadata) 
	{
		switch (metadata.Type)
		{
		case AssetType::Texture2D:
			return TextureImporter::ImportTexture(handle, metadata);
		case AssetType::TextureCube:
			return CubemapImporter::ImportCubemap(handle, metadata);
		case AssetType::Material:
			break;
		case AssetType::Mesh:
		{
			MeshImporter meshImporter;
			return meshImporter.ImportMesh(metadata.Filepath);
		}
		case AssetType::Scene:
			break;
		}

		KBR_CORE_ASSERT(false, "Unsupported asset type by AssetImporter!");
		return nullptr;
	}
}
