#include "kbrpch.h"
#include "XAudio2AudioManager.h"

#include "Kerberos/Audio/Sound.h"

namespace Kerberos
{
	/**
	 * @brief Releases audio manager resources and performs shutdown.
	 *
	 * Ensures XAudio2 resources, mastering voice, and COM state managed by the audio
	 * manager are cleaned up before destruction.
	 */
	XAudio2AudioManager::~XAudio2AudioManager() 
	{
		XAudio2AudioManager::Shutdown();
	}

	/**
	 * @brief Initializes the XAudio2 audio subsystem and prepares a mastering voice.
	 *
	 * Initializes the COM library for multithreaded use, creates the XAudio2 engine instance,
	 * and creates a mastering voice used for audio output. When built with KBR_DEBUG, applies
	 * XAUDIO2 debug configuration to the engine. Logs success on completion.
	 *
	 * @throws std::runtime_error If COM initialization, XAudio2 creation, or mastering voice creation fails.
	 */
	void XAudio2AudioManager::Init() 
	{
		HRESULT res = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
		if (FAILED(res))
		{
			KBR_CORE_ERROR("Failed to initialize COM library for XAudio2! HRESULT: {0}", res);
			KBR_CORE_ASSERT(false, "Failed to initialize COM library for XAudio2!");
			throw std::runtime_error("Failed to initialize COM library for XAudio2!");
		}

		res = XAudio2Create(&m_XAudio2, 0, XAUDIO2_USE_DEFAULT_PROCESSOR);
		if (FAILED(res))
		{
			KBR_CORE_ERROR("Failed to create XAudio2 instance! HRESULT: {0}", res);
			KBR_CORE_ASSERT(false, "Failed to create XAudio2 instance!");
			throw std::runtime_error("Failed to create XAudio2 instance!");
		}

		res = m_XAudio2->CreateMasteringVoice(&m_MasteringVoice);
		if (FAILED(res))
		{
			KBR_CORE_ERROR("Failed to create XAudio2 mastering voice! HRESULT: {0}", res);
			KBR_CORE_ASSERT(false, "Failed to create XAudio2 mastering voice!");
			throw std::runtime_error("Failed to create XAudio2 mastering voice!");
		}

#ifdef KBR_DEBUG
		XAUDIO2_DEBUG_CONFIGURATION debugConfig;
		debugConfig.TraceMask = XAUDIO2_LOG_ERRORS;
		debugConfig.BreakMask = XAUDIO2_LOG_ERRORS;
		debugConfig.LogThreadID = TRUE;
		debugConfig.LogFileline = TRUE;
		debugConfig.LogFunctionName = TRUE;
		debugConfig.LogTiming = TRUE;
		m_XAudio2->SetDebugConfiguration(&debugConfig);

#endif

		KBR_CORE_INFO("XAudio2AudioManager initialized successfully.");
	}


	/**
	 * @brief Updates playback state and cleans up finished source voices.
	 *
	 * Iterates through tracked playing audio source voices, stops and destroys any voice that has no queued buffers, removes its entry from the playing-audio map, and logs the outcome. Leaves voices with queued buffers unchanged.
	 */
	void XAudio2AudioManager::Update() 
	{
		for (auto it = m_PlayingAudios.begin(); it != m_PlayingAudios.end(); ) 
		{
			IXAudio2SourceVoice* sourceVoice = it->second;
			XAUDIO2_VOICE_STATE state;
			sourceVoice->GetState(&state);
			if (state.BuffersQueued == 0) 
			{
				const HRESULT res = sourceVoice->Stop();
				if (FAILED(res)) 
				{
					KBR_CORE_ERROR("Failed to stop source voice for audio: {0}", it->first.string());
				}
				sourceVoice->DestroyVoice();
				KBR_CORE_TRACE("Finished playing audio: {0}", it->first.string());
				it = m_PlayingAudios.erase(it);
			} 
			else 
			{
				++it;
			}
		}
	}

	/**
	 * @brief Releases XAudio2 resources and uninitializes COM for the audio manager.
	 *
	 * Destroys the mastering voice if present, releases the XAudio2 instance, clears internal pointers,
	 * and calls CoUninitialize to uninitialize the COM library.
	 */
	void XAudio2AudioManager::Shutdown() 
	{
		if (m_MasteringVoice)
		{
			m_MasteringVoice->DestroyVoice();
			m_MasteringVoice = nullptr;
		}
		if (m_XAudio2) 
		{
			m_XAudio2->Release();
			m_XAudio2 = nullptr;
		}
		CoUninitialize();
	}

	/**
	 * Loads an audio file, creates a Sound for PCM WAV files, and registers its ID-to-filepath mapping.
	 *
	 * Detects the audio format of `filepath`. If the file is PCM (WAV), creates a Sound using the file stem
	 * as the name, stores the mapping from the Sound's UUID to `filepath`, and returns a Ref to the created Sound.
	 * For unsupported or unimplemented formats, returns nullptr.
	 *
	 * @param filepath Filesystem path to the audio file to load.
	 * @return Ref<Sound> Reference to the created Sound on success, or `nullptr` if the format is unsupported or loading failed.
	 */
	Ref<Sound> XAudio2AudioManager::Load(const std::filesystem::path& filepath) 
	{
		const AudioFormat format = DetectAudioFormat(filepath);
		if (format == AudioFormat::FormatUnknown) 
		{
			KBR_CORE_ERROR("Unsupported audio format for file: {0}", filepath.string());
			return nullptr;
		}
		if (format == AudioFormat::FormatPcm) 
		{
			LoadWavFile(filepath);

			const std::string soundName = filepath.stem().string();

			Sound sound{ soundName };
			const UUID soundUUID = sound.GetSoundID();

			m_SoundUUIDToFilepath[soundUUID] = filepath;

			return CreateRef<Sound>(sound);
		}

		KBR_CORE_ERROR("Audio format not implemented for file: {0}", filepath.string());
		return nullptr;
	}

	/**
	 * Plays a previously loaded WAV file identified by its filesystem path.
	 *
	 * Looks up the WAV data that must have been loaded earlier; if the file is not loaded
	 * or contains no audio data, the function logs an error and returns. Otherwise it
	 * creates an XAudio2 source voice, submits the audio buffer, starts playback, and
	 * records the voice in the manager's playing-audio map for later control and cleanup.
	 *
	 * @param filepath Filesystem path to the WAV file that was loaded into the manager.
	 */
	void XAudio2AudioManager::Play(const std::filesystem::path& filepath) 
	{
		const auto it = m_LoadedWAVs.find(filepath);
		if (it == m_LoadedWAVs.end()) {
			KBR_CORE_ERROR("WAV file not loaded: {0}", filepath.string());
			return;
		}

		if (const auto existingIt = m_PlayingAudios.find(filepath); existingIt != m_PlayingAudios.end())
		{
			const auto& existingAudio = existingIt->second;
			existingAudio->Stop();
			existingAudio->DestroyVoice();
			m_PlayingAudios.erase(existingIt);
			KBR_CORE_WARN("Stopping sound before playing it: {0}", filepath.string());
		}

		const AudioData& soundData = it->second;
		IXAudio2SourceVoice* sourceVoice;

		if (soundData.buffer.empty()) {
			KBR_CORE_ERROR("WAV file has no audio data: {0}", filepath.string());
			return;
		}

		HRESULT res = m_XAudio2->CreateSourceVoice(&sourceVoice, &soundData.wfx);
		if (FAILED(res)) {
			KBR_CORE_ERROR("Failed to create source voice for WAV file: {0}", filepath.string());
			return;
		}
		XAUDIO2_BUFFER buffer = {};
		buffer.AudioBytes = static_cast<uint32_t>(soundData.buffer.size());
		buffer.pAudioData = soundData.buffer.data();
		buffer.Flags = XAUDIO2_END_OF_STREAM;

		res = sourceVoice->SubmitSourceBuffer(&buffer);
		if (FAILED(res)) {
			KBR_CORE_ERROR("Failed to submit source buffer for WAV file: {0}", filepath.string());
			sourceVoice->DestroyVoice();
			return;
		}

		res = sourceVoice->Start();
		if (FAILED(res)) {
			KBR_CORE_ERROR("Failed to start source voice for WAV file: {0}", filepath.string());
			sourceVoice->DestroyVoice();
			return;
		}

		m_PlayingAudios[filepath] = sourceVoice;

		KBR_CORE_INFO("Playing WAV file: {0}", filepath.string());
	}

	/**
	 * Plays the sound associated with the given sound UUID.
	 *
	 * If the UUID is not mapped to a loaded file, the function logs an error and returns without playing.
	 *
	 * @param soundID UUID of the sound to play.
	 */
	void XAudio2AudioManager::Play(const UUID& soundID) 
	{
		const auto filepath = m_SoundUUIDToFilepath.find(soundID);
		if (filepath == m_SoundUUIDToFilepath.end()) 
		{
			KBR_CORE_ERROR("Sound ID not found: {0}", static_cast<uint64_t>(soundID));
			return;
		}

		Play(filepath->second);
	}

	/**
	 * Stops playback of the sound identified by `soundID` and releases its associated source voice.
	 *
	 * If `soundID` is not mapped to a loaded filepath or the sound is not currently playing, the function logs an error and returns without side effects. On success, the source voice is stopped, destroyed, and the playing entry is removed.
	 *
	 * @param soundID UUID of the sound to stop.
	 */
	void XAudio2AudioManager::Stop(const UUID& soundID) 
	{
		const auto filepath = m_SoundUUIDToFilepath.find(soundID);
		if (filepath == m_SoundUUIDToFilepath.end()) 
		{
			KBR_CORE_ERROR("Sound ID not found: {0}", static_cast<uint64_t>(soundID));
			return;
		}

		const auto it = m_PlayingAudios.find(filepath->second);
		if (it == m_PlayingAudios.end()) 
		{
			KBR_CORE_ERROR("Sound is not currently playing: {0}", filepath->second.string());
			return;
		}

		IXAudio2SourceVoice* sourceVoice = it->second;
		const HRESULT res = sourceVoice->Stop();
		if (FAILED(res))
		{
			KBR_CORE_ERROR("Failed to stop source voice for audio: {0}", filepath->second.string());
			return;
		}

		sourceVoice->DestroyVoice();
		m_PlayingAudios.erase(it);
	}

	/**
	 * @brief Increases the playback volume for a currently playing sound by the specified delta.
	 *
	 * If the sound ID is unknown or the sound is not currently playing, the function logs an error and returns without changing volume.
	 * If setting the new volume fails, the function logs an error.
	 *
	 * @param soundID UUID of the sound whose volume will be increased.
	 * @param delta Amount to add to the current volume (linear scale).
	 */
	void XAudio2AudioManager::IncreaseVolume(const UUID& soundID, const float delta)
	{
		const auto filepath = m_SoundUUIDToFilepath.find(soundID);
		if (filepath == m_SoundUUIDToFilepath.end()) 
		{
			KBR_CORE_ERROR("Sound ID not found: {0}", static_cast<uint64_t>(soundID));
			return;
		}

		const auto audio = m_PlayingAudios.find(filepath->second);
		if (audio == m_PlayingAudios.end()) 
		{
			KBR_CORE_ERROR("You can only increase the volume of a sound currently playing: {0}", filepath->second.string());
			return;
		}

		float currentVolume = 0;
		audio->second->GetVolume(&currentVolume);

		if (const HRESULT res = audio->second->SetVolume(currentVolume + delta); FAILED(res)) 
		{
			KBR_CORE_ERROR("Failed to increase volume for sound: {0}", filepath->second.string());
		}
	}

	/**
	 * @brief Decreases the playback volume of a currently playing sound.
	 *
	 * Looks up the sound by its UUID and subtracts `delta` from the sound's current volume.
	 * If the UUID is unknown or the sound is not playing, the call has no effect.
	 *
	 * @param soundID UUID of the sound whose volume will be decreased.
	 * @param delta Amount to subtract from the current volume (linear scale).
	 */
	void XAudio2AudioManager::DecreaseVolume(const UUID& soundID, const float delta)
	{
		const auto filepath = m_SoundUUIDToFilepath.find(soundID);
		if (filepath == m_SoundUUIDToFilepath.end())
		{
			KBR_CORE_ERROR("Sound ID not found: {0}", static_cast<uint64_t>(soundID));
			return;
		}

		const auto audio = m_PlayingAudios.find(filepath->second);
		if (audio == m_PlayingAudios.end())
		{
			KBR_CORE_ERROR("You can only decrease the volume of a sound currently playing: {0}", filepath->second.string());
			return;
		}

		float currentVolume = 0;
		audio->second->GetVolume(&currentVolume);

		if (const HRESULT res = audio->second->SetVolume(currentVolume - delta); FAILED(res))
		{
			KBR_CORE_ERROR("Failed to decrease volume for sound: {0}", filepath->second.string());
		}
	}

	/**
	 * @brief Sets the playback volume for a currently playing sound.
	 *
	 * Sets the volume for the sound identified by `soundID` if that sound is currently playing.
	 * The value is linear where 1.0 is the original (unmodified) volume and 0.0 is silence.
	 * If `soundID` is not known or the sound is not currently playing, the call has no effect.
	 *
	 * @param soundID Unique identifier of the sound.
	 * @param volume Linear volume multiplier to apply to the sound (1.0 = original, 0.0 = mute).
	 */
	void XAudio2AudioManager::SetVolume(const UUID& soundID, const float volume)
	{
		const auto filepath = m_SoundUUIDToFilepath.find(soundID);
		if (filepath == m_SoundUUIDToFilepath.end())
		{
			KBR_CORE_ERROR("Sound ID not found: {0}", static_cast<uint64_t>(soundID));
			return;
		}

		const auto audio = m_PlayingAudios.find(filepath->second);
		if (audio == m_PlayingAudios.end())
		{
			KBR_CORE_ERROR("You can only set the volume of a sound currently playing: {0}", filepath->second.string());
			return;
		}

		if (const HRESULT res = audio->second->SetVolume(volume); FAILED(res))
		{
			KBR_CORE_ERROR("Failed to set volume for sound: {0}", filepath->second.string());
		}
	}

	/**
	 * @brief Resets the playback volume for a sound to the default level.
	 *
	 * @param soundID UUID identifying the sound whose volume will be set to 1.0f.
	 */
	void XAudio2AudioManager::ResetVolume(const UUID& soundID)
	{
		SetVolume(soundID, 1.0f);
	}

	/**
	 * @brief Mutes the sound associated with the given ID by setting its volume to 0.
	 *
	 * @param soundID UUID of the sound to mute.
	 */
	void XAudio2AudioManager::Mute(const UUID& soundID)
	{
		SetVolume(soundID, 0.0f);
	}


	/**
	 * @brief Determines the audio format of a file by its extension.
	 *
	 * @param filepath Filesystem path to the audio file whose format should be detected.
	 * @return AudioFormat The detected format: `FormatPcm` for `.wav`, `FormatAdpcm` for `.adpcm`, `FormatIeeeFloat` for `.f32`, or `FormatUnknown` if the extension is unrecognized.
	 */
	AudioFormat XAudio2AudioManager::DetectAudioFormat(const std::filesystem::path& filepath) 
	{
		const std::string extension = filepath.extension().string();
		if (extension == ".wav" || extension == ".WAV") 
		{
			return AudioFormat::FormatPcm;
		}
		if (extension == ".adpcm" || extension == ".ADPCM") 
		{
			return AudioFormat::FormatAdpcm;
		}
		if (extension == ".f32" || extension == ".F32") 
		{
			return AudioFormat::FormatIeeeFloat;
		}
		return AudioFormat::FormatUnknown;
	}

	/**
	 * @brief Loads a PCM WAV file from disk and stores its parsed audio data for playback.
	 *
	 * Parses the RIFF/WAVE file at the given path, extracts the 'fmt ' chunk into a WAVEFORMATEX
	 * structure and the 'data' chunk into a raw byte buffer, then stores the result in
	 * m_LoadedWAVs keyed by the provided filepath.
	 *
	 * If the file cannot be opened, is not a valid RIFF/WAVE file, or is missing either the
	 * 'fmt ' or 'data' chunk (or the data chunk is empty), the function logs an error and
	 * returns without modifying m_LoadedWAVs.
	 *
	 * @param filepath Filesystem path to the WAV file to load. Only PCM/WAVE formats with a
	 *                 standard 'fmt ' chunk are supported; extra format bytes (chunkSize > 16)
	 *                 are skipped and cbSize is set to 0 for the parsed format.
	 */
	void XAudio2AudioManager::LoadWavFile(const std::filesystem::path& filepath) 
	{
		std::ifstream file(filepath, std::ios::binary);
		if (!file) {
			KBR_CORE_ERROR("Failed to open WAV file: {0}", filepath.string());
			return;
		}

		char chunkId[4];
		file.read(chunkId, 4);
		if (strncmp(chunkId, "RIFF", 4) != 0) {
			KBR_CORE_ERROR("Invalid WAV file (missing RIFF): {0}", filepath.string());
			return;
		}

		DWORD chunkSize;

		file.read(reinterpret_cast<char*>(&chunkSize), 4);

		file.read(chunkId, 4);
		if (strncmp(chunkId, "WAVE", 4) != 0) {
			KBR_CORE_ERROR("Invalid WAV file (missing WAVE): {0}", filepath.string());
			return;
		}

		AudioData soundData;
		bool foundFmt = false;
		bool foundData = false;

		// Parse chunks
		while (file.read(chunkId, 4)) {
			file.read(reinterpret_cast<char*>(&chunkSize), 4);

			if (strncmp(chunkId, "fmt ", 4) == 0) {
				// Read format chunk
				file.read(reinterpret_cast<char*>(&soundData.wfx.wFormatTag), 2);
				file.read(reinterpret_cast<char*>(&soundData.wfx.nChannels), 2);
				file.read(reinterpret_cast<char*>(&soundData.wfx.nSamplesPerSec), 4);
				file.read(reinterpret_cast<char*>(&soundData.wfx.nAvgBytesPerSec), 4);
				file.read(reinterpret_cast<char*>(&soundData.wfx.nBlockAlign), 2);
				file.read(reinterpret_cast<char*>(&soundData.wfx.wBitsPerSample), 2);

				// Set cbSize to 0 for PCM format
				soundData.wfx.cbSize = 0;

				// Skip any extra format bytes
				if (chunkSize > 16) {
					file.seekg(chunkSize - 16, std::ios::cur);
				}

				foundFmt = true;
				KBR_CORE_TRACE("WAV Format - Channels: {0}, SampleRate: {1}, BitsPerSample: {2}",
					soundData.wfx.nChannels, soundData.wfx.nSamplesPerSec, soundData.wfx.wBitsPerSample);
			}
			else if (strncmp(chunkId, "data", 4) == 0) {
				// Read audio data
				soundData.buffer.resize(chunkSize);
				file.read(reinterpret_cast<char*>(soundData.buffer.data()), chunkSize);
				foundData = true;
				KBR_CORE_TRACE("WAV Data - Size: {0} bytes", chunkSize);
				break; // Data chunk is typically the last one we need
			}
			else {
				// Skip unknown chunk
				file.seekg(chunkSize, std::ios::cur);
			}
		}

		if (!foundFmt) {
			KBR_CORE_ERROR("WAV file missing 'fmt ' chunk: {0}", filepath.string());
			return;
		}

		if (!foundData) {
			KBR_CORE_ERROR("WAV file missing 'data' chunk: {0}", filepath.string());
			return;
		}

		if (soundData.buffer.empty()) {
			KBR_CORE_ERROR("WAV file has empty audio data: {0}", filepath.string());
			return;
		}

		m_LoadedWAVs[filepath] = std::move(soundData);
	}
}