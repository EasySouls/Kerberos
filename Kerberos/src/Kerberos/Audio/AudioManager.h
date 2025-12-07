#pragma once

#include "Kerberos/Core.h"
#include "Sound.h"

#include <filesystem>


/**
 * Abstract interface for audio management and playback.
 *
 * Implementations manage sound loading, playback, and per-sound volume/mute controls.
 */
 
/**
 * Initialize the audio subsystem and any backend-specific resources.
 */
 
/**
 * Update the audio subsystem (called per-frame or periodically).
 */
 
/**
 * Shutdown the audio subsystem and release resources.
 */
 
/**
 * Load a sound from disk and return a reference to the loaded Sound.
 * @param filepath Filesystem path to the sound asset to load.
 * @returns A Ref<Sound> referencing the loaded sound.
 */
 
/**
 * Play a sound from the given filesystem path.
 * @param filepath Filesystem path to the sound asset to play.
 */
 
/**
 * Play a previously loaded sound identified by its UUID.
 * @param soundID UUID of the sound to play.
 */
 
/**
 * Stop playback of a sound identified by its UUID.
 * @param soundID UUID of the sound to stop.
 */
 
/**
 * Increase the volume of the specified sound by `delta`.
 * @param soundID UUID of the sound whose volume will be increased.
 * @param delta Amount to increase the volume by.
 */
 
/**
 * Decrease the volume of the specified sound by `delta`.
 * @param soundID UUID of the sound whose volume will be decreased.
 * @param delta Amount to decrease the volume by.
 */
 
/**
 * Set the volume for the specified sound.
 * @param soundID UUID of the sound whose volume will be set.
 * @param volume New volume level (typically in the range 0.0 to 1.0).
 */
 
/**
 * Reset the volume of the specified sound to its default level.
 * @param soundID UUID of the sound whose volume will be reset.
 */
 
/**
 * Mute the specified sound.
 * @param soundID UUID of the sound to mute.
 */
 
/**
 * Create and return a concrete AudioManager instance.
 * @returns Pointer to a newly created AudioManager instance; the caller takes ownership and is responsible for deleting it.
 */
namespace Kerberos
{
	class AudioManager
	{
	public:
		AudioManager() = default;
		virtual ~AudioManager() = default;

		AudioManager(const AudioManager& other) = default;
		AudioManager(AudioManager&& other) noexcept = default;
		AudioManager& operator=(const AudioManager& other) = default;
		AudioManager& operator=(AudioManager&& other) noexcept = default;

		virtual void Init() = 0;
		virtual void Update() = 0;
		virtual void Shutdown() = 0;

		virtual Ref<Sound> Load(const std::filesystem::path& filepath) = 0;
		virtual void Play(const std::filesystem::path& filepath) = 0;
		virtual void Play(const UUID& soundID) = 0;
		virtual void Stop(const UUID& soundID) = 0;

		virtual void IncreaseVolume(const UUID& soundID, float delta) = 0;
		virtual void DecreaseVolume(const UUID& soundID, float delta) = 0;
		virtual void SetVolume(const UUID& soundID, float volume) = 0;
		virtual void ResetVolume(const UUID& soundID) = 0;
		virtual void Mute(const UUID& soundID) = 0;

		static AudioManager* Create();
	};
}