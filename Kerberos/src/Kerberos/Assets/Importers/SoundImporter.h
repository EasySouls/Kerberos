#pragma once

#include "Kerberos/Assets/Asset.h"
#include "Kerberos/Assets/AssetMetadata.h"
#include "Kerberos/Audio/Sound.h"

namespace Kerberos
{
	class SoundImporter
	{
	public:
		static Ref<Sound> ImportSound(AssetHandle handle, const AssetMetadata& metadata);
		static Ref<Sound> ImportSound(const std::filesystem::path& filepath);
	};
}
