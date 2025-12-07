#pragma once

#include "Kerberos/Assets/Asset.h"
#include "Kerberos/Assets/AssetMetadata.h"
#include "Kerberos/Audio/Sound.h"

/**
 * Import a sound asset identified by an asset handle and its metadata.
 * @param handle Asset handle that identifies the source asset to import.
 * @param metadata Metadata describing the asset (format, settings, source info) used during import.
 * @returns Reference to the imported Sound.
 */

/**
 * Import a sound asset from a filesystem path.
 * @param filepath Filesystem path to the sound file to import.
 * @returns Reference to the imported Sound.
 */
namespace Kerberos
{
	class SoundImporter
	{
	public:
		static Ref<Sound> ImportSound(AssetHandle handle, const AssetMetadata& metadata);
		static Ref<Sound> ImportSound(const std::filesystem::path& filepath);
	};
}