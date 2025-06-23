#pragma once
#include "Kerberos/Renderer/Mesh.h"
#include "Kerberos/Renderer/Texture.h"
#include "Kerberos/Assets/Asset.h"

namespace Kerberos
{
	class AssetManagerBase
	{
	public:
		virtual ~AssetManagerBase() = default;

		virtual Ref<Asset> GetAsset(AssetHandle handle) = 0;
	};
}
