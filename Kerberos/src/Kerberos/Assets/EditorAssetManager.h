#pragma once

#include "AssetRegistry.h"
#include "Kerberos/Assets/AssetManagerBase.h"

namespace Kerberos
{

	class EditorAssetManager : public AssetManagerBase
	{
	public:
		Ref<Asset> GetAsset(AssetHandle handle) override;

		bool IsAssetHandleValid(AssetHandle handle) override;
		bool IsAssetLoaded(AssetHandle handle) override;

		const AssetMetadata& GetMetadata(AssetHandle handle) const;

		void SerializeAssetRegistry();
		bool DeserializeAssetRegistry();

	private:
		AssetMap m_LoadedAssets;
		AssetRegistry m_AssetRegistry;
	};
}