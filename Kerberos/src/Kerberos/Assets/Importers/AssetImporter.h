#pragma once

#include "Kerberos/Assets/Asset.h"
#include "Kerberos/Assets/AssetMetadata.h"

#include <future>

namespace Kerberos
{
	class AssetImporter
	{
	public:
		static Ref<Asset> ImportAsset(AssetHandle handle, const AssetMetadata& metadata);

		static std::future<Ref<Asset>> ImportAssetAsync(AssetHandle handle, const AssetMetadata& metadata)
		{
			return std::async(std::launch::async, ImportAsset, handle, metadata);
		}
	};
}