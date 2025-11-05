#pragma once

#include "Kerberos/Audio/AudioManager.h"

#include <xaudio2.h>
#include <xaudio2fx.h>

#include <unordered_map>


namespace Kerberos
{
	enum class AudioFormat
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
