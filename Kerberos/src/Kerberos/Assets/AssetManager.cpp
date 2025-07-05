#include "kbrpch.h"
#include "AssetManager.h"

namespace Kerberos
{
	static int s_DefaultTextureCount = 0;

	Ref<Texture2D> AssetManager::GetDefaultTexture2D()
	{
		TextureSpecification spec;
		spec.Width = 1;
		spec.Height = 1;

		Ref<Texture2D> defaultTexture = Texture2D::Create(spec);
		defaultTexture->SetDebugName("Default Texture " + std::to_string(s_DefaultTextureCount++));
		uint32_t data = 0xFFFFFFFF;
		defaultTexture->SetData(&data, sizeof(uint32_t));
		return defaultTexture;
	}

	Ref<Mesh> AssetManager::GetDefaultCubeMesh()
	{
		return Mesh::CreateCube(1.0f);
	}
}