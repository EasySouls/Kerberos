#include "kbrpch.h"
#include "AssetManager.h"

namespace Kerberos
{
	Ref<Texture2D> AssetManager::GetDefaultTexture2D()
	{
		const Ref<Texture2D> defaultTexture = Texture2D::Create(1, 1);
		uint32_t data = 0xFFFFFFFF;
		defaultTexture->SetData(&data, sizeof(uint32_t));
		return defaultTexture;
	}

	Ref<Mesh> AssetManager::GetDefaultCubeMesh()
	{
		return Mesh::CreateCube(1.0f);
	}
}