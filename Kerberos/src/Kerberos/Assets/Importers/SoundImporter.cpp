#include "kbrpch.h"
#include "SoundImporter.h"

#include "Kerberos/Application.h"
#include "Kerberos/Audio/AudioManager.h"

namespace Kerberos
{
	Ref<Sound> SoundImporter::ImportSound(AssetHandle, const AssetMetadata& metadata) 
	{
		return ImportSound(metadata.Filepath);
	}

	Ref<Sound> SoundImporter::ImportSound(const std::filesystem::path& filepath) 
	{
		return Application::Get().GetAudioManager()->Load(filepath);
	}
}
