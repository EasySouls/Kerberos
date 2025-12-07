#pragma once

#include "Kerberos/Audio/AudioManager.h"

#include <xaudio2.h>
#include <xaudio2fx.h>

#include <unordered_map>


/**
 * @enum AudioFormat
 * @brief Supported audio data formats.
 *
 * Enumerates recognized container/codec types for loaded audio data.
 */

/**
 * @struct AudioData
 * @brief Holds decoded audio format metadata and raw sample bytes.
 *
 * Contains a WAVEFORMATEX describing the sample format, a contiguous buffer
 * of raw audio bytes, and the detected AudioFormat.
 */

/**
 * @class XAudio2AudioManager
 * @brief Platform implementation of the AudioManager using XAudio2.
 *
 * Manages initialization and shutdown of the XAudio2 engine, loading of WAV
 * assets, playback control, and per-sound volume management.
 */

/**
 * @brief Initialize the XAudio2 engine and related resources.
 */

/**
 * @brief Perform per-frame audio manager work (e.g., cleanup finished voices).
 */

/**
 * @brief Shutdown the XAudio2 engine and release all audio resources.
 */

/**
 * @brief Load a sound from disk and register it with the audio manager.
 * @param filepath Filesystem path to the audio file to load.
 * @returns Ref<Sound> Reference to the loaded sound resource.
 */

/**
 * @brief Play a sound identified by a filesystem path.
 * @param filepath Filesystem path of a previously loaded sound.
 */

/**
 * @brief Play a sound identified by its UUID.
 * @param soundID Unique identifier of the sound to play.
 */

/**
 * @brief Stop playback of a sound identified by its UUID.
 * @param soundID Unique identifier of the sound to stop.
 */

/**
 * @brief Increase the playback volume of a sound by a delta.
 * @param soundID Unique identifier of the sound.
 * @param delta Amount to add to the current volume.
 */

/**
 * @brief Decrease the playback volume of a sound by a delta.
 * @param soundID Unique identifier of the sound.
 * @param delta Amount to subtract from the current volume.
 */

/**
 * @brief Set the playback volume of a sound.
 * @param soundID Unique identifier of the sound.
 * @param volume Absolute volume level to set.
 */

/**
 * @brief Reset the playback volume of a sound to its default value.
 * @param soundID Unique identifier of the sound.
 */

/**
 * @brief Mute or unmute a sound.
 * @param soundID Unique identifier of the sound.
 */

/**
 * @brief Detect the audio format for a file based on its path or header.
 * @param filepath Filesystem path to the audio file to inspect.
 * @returns AudioFormat The detected audio format enum value.
 */

/**
 * @brief Load WAV file contents into the manager's in-memory structures.
 * @param filepath Filesystem path to the WAV file to load.
 */
namespace Kerberos
{
	enum class AudioFormat : uint8_t
	{
		FormatUnknown,
		FormatPcm,
		FormatAdpcm,
		FormatIeeeFloat
	};

	struct AudioData 
	{
		WAVEFORMATEX wfx;
		std::vector<uint8_t> buffer;
		AudioFormat format = AudioFormat::FormatUnknown;

		AudioData() 
		{
			memset(&wfx, 0, sizeof(WAVEFORMATEX));
		}
	};

	class XAudio2AudioManager : public AudioManager
	{
	public:
		XAudio2AudioManager() = default;
		~XAudio2AudioManager() override;

		// TODO: Implement proper move behaviour, since we are using COM pointers.
		XAudio2AudioManager(const XAudio2AudioManager& other) = delete;
		XAudio2AudioManager(XAudio2AudioManager&& other) noexcept = default;
		XAudio2AudioManager& operator=(const XAudio2AudioManager& other) = delete;
		XAudio2AudioManager& operator=(XAudio2AudioManager&& other) noexcept = default;

		void Init() override;
		void Update() override;
		void Shutdown() override;

		Ref<Sound> Load(const std::filesystem::path& filepath) override;
		void Play(const std::filesystem::path& filepath) override;
		void Play(const UUID& soundID) override;
		void Stop(const UUID& soundID) override;

		void IncreaseVolume(const UUID& soundID, float delta) override;
		void DecreaseVolume(const UUID& soundID, float delta) override;
		void SetVolume(const UUID& soundID, float volume) override;
		void ResetVolume(const UUID& soundID) override;
		void Mute(const UUID& soundID) override;

	private:
		static AudioFormat DetectAudioFormat(const std::filesystem::path& filepath);

		void LoadWavFile(const std::filesystem::path& filepath);

	private:
		IXAudio2* m_XAudio2 = nullptr;
		IXAudio2MasteringVoice* m_MasteringVoice = nullptr;

		std::unordered_map<std::filesystem::path, AudioData> m_LoadedWAVs;
		std::unordered_map<UUID, std::filesystem::path> m_SoundUUIDToFilepath;
		std::unordered_map<std::filesystem::path, IXAudio2SourceVoice*> m_PlayingAudios;
	};
}