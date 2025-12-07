#include "kbrpch.h"
#include "SoundImporter.h"

#include "Kerberos/Application.h"
#include "Kerberos/Audio/AudioManager.h"

namespace Kerberos
{
	/**
	 * @brief Imports a sound asset using the file path contained in the asset metadata.
	 *
	 * @param metadata Asset metadata containing the file path to load.
	 * @return Ref<Sound> Reference to the loaded Sound.
	 */
	Ref<Sound> SoundImporter::ImportSound(AssetHandle, const AssetMetadata& metadata) 
	{
		return ImportSound(metadata.Filepath);
	}

	/**
	 * @brief Loads a Sound from the given filesystem path.
	 *
	 * @param filepath Filesystem path to the sound file to load.
	 * @return Ref<Sound> Reference to the loaded Sound object.
	 */
	Ref<Sound> SoundImporter::ImportSound(const std::filesystem::path& filepath) 
	{
		return Application::Get().GetAudioManager()->Load(filepath);
	}
}