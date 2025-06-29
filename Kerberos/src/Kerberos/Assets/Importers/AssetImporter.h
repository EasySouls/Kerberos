#pragma once

#include "Kerberos/Assets/Asset.h"
#include "Kerberos/Assets/AssetMetadata.h"

namespace Kerberos
{
	class AssetImporter
	{
	public:
		static Ref<Asset> ImportAsset(AssetHandle handle, const AssetMetadata& metadata);
	};
}