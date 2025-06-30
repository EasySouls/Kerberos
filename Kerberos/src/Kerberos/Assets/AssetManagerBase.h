#pragma once
#include "Kerberos/Renderer/Mesh.h"
#include "Kerberos/Renderer/Texture.h"
#include "Kerberos/Assets/Asset.h"

#include <map>


namespace Kerberos
{
	using AssetMap = std::map<AssetHandle, Ref<Asset>>;

	class AssetManagerBase
	{
	public:
		virtual ~AssetManagerBase() = default;

		virtual Ref<Asset> GetAsset(AssetHandle handle) = 0;

		virtual bool IsAssetHandleValid(AssetHandle handle) const = 0;
		virtual bool IsAssetLoaded(AssetHandle handle) const = 0;

		virtual AssetType GetAssetType(AssetHandle handle) const = 0;
	};
}
