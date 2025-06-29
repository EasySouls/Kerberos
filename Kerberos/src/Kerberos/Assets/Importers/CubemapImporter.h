#pragma once
#include "Kerberos/Assets/AssetMetadata.h"
#include "Kerberos/Renderer/TextureCube.h"

namespace Kerberos
{
	struct CubemapDescriptor
	{
		std::string Name;

		std::filesystem::path RightPath;
		std::filesystem::path LeftPath;
		std::filesystem::path TopPath;
		std::filesystem::path BottomPath;
		std::filesystem::path FrontPath;
		std::filesystem::path BackPath;

		bool IsSRGB;
	};

	class CubemapImporter
	{
	public:
		static Ref<TextureCube> ImportCubemap(AssetHandle handle, const AssetMetadata& metadata);
		static Ref<TextureCube> ImportCubemap(const std::filesystem::path& filepath);
	};
}
