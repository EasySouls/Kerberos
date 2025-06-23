#include "kbrpch.h"
#include "AssetManager.h"

namespace Kerberos
{
	Ref<Texture2D> AssetManager::GetDefaultTexture2D()
	{
		TextureSpecification spec;
		spec.Width = 1;
		spec.Height = 1;

		Ref<Texture2D> defaultTexture = Texture2D::Create(spec);
		uint32_t data = 0xFFFFFFFF;
		defaultTexture->SetData(&data, sizeof(uint32_t));
		return defaultTexture;
	}

	Ref<Mesh> AssetManager::GetDefaultCubeMesh()
	{
		return Mesh::CreateCube(1.0f);
	}
}