#include "kbrpch.h"
#include "RuntimeAssetManager.h"

namespace Kerberos
{
	Ref<Asset> RuntimeAssetManager::GetAsset(AssetHandle handle) 
	{
		throw std::runtime_error("RuntimeAssetManager::GetAsset is not implemented yet!");
	}

	bool RuntimeAssetManager::IsAssetHandleValid(AssetHandle handle) 
	{
		throw std::runtime_error("RuntimeAssetManager::IsAssetHandleValid is not implemented yet!");
	}

	bool RuntimeAssetManager::IsAssetLoaded(AssetHandle handle)
	{
		throw std::runtime_error("RuntimeAssetManager::IsAssetLoaded is not implemented yet!");
	}
}
