#pragma once

#include "AssetRegistry.h"
#include "Kerberos/Assets/AssetManagerBase.h"

namespace Kerberos
{

	class EditorAssetManager : public AssetManagerBase
	{
	public:
		Ref<Asset> GetAsset(AssetHandle handle) override;

		bool IsAssetHandleValid(AssetHandle handle) const override;
		bool IsAssetLoaded(AssetHandle handle) const override;

		AssetType GetAssetType(AssetHandle handle) const override;

		AssetHandle ImportAsset(const std::filesystem::path& filepath);

		const AssetMetadata& GetMetadata(AssetHandle handle) const;

		void SerializeAssetRegistry();
		bool DeserializeAssetRegistry();

		const AssetRegistry& GetAssetRegistry() const { return m_AssetRegistry; }

	private:
		AssetMap m_LoadedAssets;
		AssetRegistry m_AssetRegistry;
	};
}