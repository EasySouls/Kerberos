#pragma once

#include "Kerberos/Assets/AssetManagerBase.h"

namespace Kerberos
{
	class RuntimeAssetManager final : public AssetManagerBase
	{
	public:
		Ref<Asset> GetAsset(AssetHandle handle) override;

		bool IsAssetHandleValid(AssetHandle handle) const override;
		bool IsAssetLoaded(AssetHandle handle) const override;

		AssetType GetAssetType(AssetHandle handle) const override;
	};
}


