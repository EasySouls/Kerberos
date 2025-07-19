#pragma once

#include "Kerberos/Renderer/Mesh.h"
#include "Kerberos/Renderer/Texture.h"
#include "Kerberos/Assets/Asset.h"
#include "Kerberos/Project/Project.h"

namespace Kerberos
{
	class AssetManager
	{
	public:
		template<typename T>
		static Ref<T> GetAsset(const AssetHandle handle)
		{
			const Ref<Asset> asset = Project::GetActive()->GetAssetManager()->GetAsset(handle);
			return std::static_pointer_cast<T>(asset);
		}

		static AssetType GetAssetType(const AssetHandle handle)
		{
			return Project::GetActive()->GetAssetManager()->GetAssetType(handle);
		}

		static bool IsAssetHandleValid(const AssetHandle handle)
		{
			return Project::GetActive()->GetAssetManager()->IsAssetHandleValid(handle);
		}

		static Ref<Texture2D> GetDefaultTexture2D();
		static Ref<Mesh> GetDefaultCubeMesh();
	};
}
