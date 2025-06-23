#include "kbrpch.h"
#include "EditorAssetManager.h"

#include "AssetImporter.h"

namespace Kerberos
{
	Ref<Asset> EditorAssetManager::GetAsset(const AssetHandle handle) 
	{
		if (!IsAssetHandleValid(handle))
			return nullptr;

		Ref<Asset> asset = nullptr;
		if (IsAssetLoaded(handle))
		{
			asset = m_LoadedAssets.at(handle);
		}
		else
		{
			const AssetMetadata& metadata = GetMetadata(handle);
			asset = AssetImporter::ImportAsset(handle, metadata);
			if (!asset)
			{
				KBR_CORE_ERROR("Asset import failed!");
				return nullptr;
			}

			/// Save the loaded asset
			m_LoadedAssets[handle] = asset;
		}

		return asset;
	}

	bool EditorAssetManager::IsAssetHandleValid(const AssetHandle handle) 
	{
		if (!handle.IsValid())
			return false;

		return m_AssetRegistry.Contains(handle);
	}

	bool EditorAssetManager::IsAssetLoaded(const AssetHandle handle) 
	{
		return m_LoadedAssets.contains(handle);
	}

	const AssetMetadata& EditorAssetManager::GetMetadata(const AssetHandle handle) const
	{
		return m_AssetRegistry.Get(handle);
	}
}
