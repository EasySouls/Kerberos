#pragma once
#include "AssetMetadata.h"
#include "Kerberos/Renderer/Texture.h"

namespace Kerberos
{
	class TextureImporter
	{
	public:
		static Ref<Texture2D> ImportTexture(AssetHandle handle, const AssetMetadata& metadata);
	};
}
