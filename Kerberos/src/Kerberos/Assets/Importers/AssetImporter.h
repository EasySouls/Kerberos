#pragma once

#include "Kerberos/Assets/Asset.h"
#include "Kerberos/Assets/AssetMetadata.h"

#include <future>

import ThreadPool;

namespace Kerberos
{
	class AssetImporter
	{
	public:
		static void Init();

		static Ref<Asset> ImportAsset(AssetHandle handle, const AssetMetadata& metadata);

		static std::future<Ref<Asset>> ImportAssetAsync(AssetHandle handle, const AssetMetadata& metadata)
		{
			return std::async(std::launch::async, ImportAsset, handle, metadata);
		}

	private:
		inline static ThreadPool m_ThreadPool = ThreadPool(4);
	};
}