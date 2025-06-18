#pragma once
#include "Kerberos/Renderer/Mesh.h"
#include "Kerberos/Renderer/Texture.h"

namespace Kerberos
{
	class AssetManager
	{
	public:
		static Ref<Texture2D> GetDefaultTexture2D();
		static Ref<Mesh> GetDefaultCubeMesh();
	};
}
