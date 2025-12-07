#include "kbrpch.h"
#include "AssetImporter.h"

#include "CubemapImporter.h"
#include "TextureImporter.h"
#include "MeshImporter.h"
#include "SoundImporter.h"
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

	/**
	 * @brief Imports an asset according to the provided metadata and returns the created asset.
	 *
	 * Selects the appropriate importer based on metadata.Type and delegates the import operation.
	 * Asserts and returns `nullptr` if the asset type is unsupported.
	 *
	 * @param handle Handle to assign to the imported asset.
	 * @param metadata Metadata describing the asset (includes type and filepath).
	 * @return Ref<Asset> Reference to the imported asset, or `nullptr` if the asset type is unsupported.
	 */
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
		case AssetType::Sound:
			return SoundImporter::ImportSound(handle, metadata);
		}

		KBR_CORE_ASSERT(false, "Unsupported asset type by AssetImporter!");
		return nullptr;
	}
}