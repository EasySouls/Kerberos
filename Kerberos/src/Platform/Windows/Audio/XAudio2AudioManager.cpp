#include "kbrpch.h"
#include "XAudio2AudioManager.h"

namespace Kerberos
{
	XAudio2AudioManager::~XAudio2AudioManager() 
	{
		XAudio2AudioManager::Shutdown();
	}

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
		debugConfig.TraceMask = XAUDIO2_LOG_WARNINGS;
		debugConfig.BreakMask = 0;
		debugConfig.LogThreadID = TRUE;
		debugConfig.LogFileline = TRUE;
		debugConfig.LogFunctionName = TRUE;
		debugConfig.LogTiming = TRUE;
		m_XAudio2->SetDebugConfiguration(&debugConfig);
#endif

		KBR_CORE_INFO("XAudio2AudioManager initialized successfully.");
	}


	void XAudio2AudioManager::Update() {}

	void XAudio2AudioManager::Shutdown() 
	{
		m_XAudio2->Release();
		CoUninitialize();
	}

	void XAudio2AudioManager::LoadSound(const std::filesystem::path& filepath) 
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

		WAVData soundData;

		file.seekg(22); // skip to channels
		file.read((char*)&soundData.wfx.nChannels, 2);
		file.read((char*)&soundData.wfx.nSamplesPerSec, 4);
		file.seekg(34);
		file.read((char*)&soundData.wfx.wBitsPerSample, 2);

		// crude data find
		file.seekg(0, std::ios::beg);
		while (file.read(chunkId, 4)) {
			DWORD chunkSize;
			file.read((char*)&chunkSize, 4);
			if (strncmp(chunkId, "data", 4) == 0) {
				soundData.buffer.resize(chunkSize);
				file.read((char*)soundData.buffer.data(), chunkSize);
				break;
			}

			file.seekg(chunkSize, std::ios::cur);
		}

		soundData.wfx.wFormatTag = WAVE_FORMAT_PCM;
		soundData.wfx.nBlockAlign = soundData.wfx.nChannels * soundData.wfx.wBitsPerSample / 8;
		soundData.wfx.nAvgBytesPerSec = soundData.wfx.nSamplesPerSec * soundData.wfx.nBlockAlign;

		m_LoadedWAVs[filepath] = std::move(soundData);
	}

	void XAudio2AudioManager::PlaySoundW(const std::filesystem::path& filepath) 
	{
		const auto it = m_LoadedWAVs.find(filepath);
		if (it == m_LoadedWAVs.end()) {
			KBR_CORE_ERROR("WAV file not loaded: {0}", filepath.string());
			return;
		}

		const WAVData& soundData = it->second;
		IXAudio2SourceVoice* sourceVoice;

		HRESULT res = m_XAudio2->CreateSourceVoice(&sourceVoice, &soundData.wfx);
		if (FAILED(res)) {
			KBR_CORE_ERROR("Failed to create source voice for WAV file: {0}", filepath.string());
			return;
		}
		XAUDIO2_BUFFER buffer = {};
		buffer.AudioBytes = static_cast<UINT32>(soundData.buffer.size());
		buffer.pAudioData = soundData.buffer.data();
		buffer.Flags = XAUDIO2_END_OF_STREAM;

		res = sourceVoice->SubmitSourceBuffer(&buffer);
		if (FAILED(res)) {
			KBR_CORE_ERROR("Failed to submit source buffer for WAV file: {0}", filepath.string());
			//sourceVoice->DestroyVoice();
		}

		res = sourceVoice->Start();
		if (FAILED(res)) {
			KBR_CORE_ERROR("Failed to start source voice for WAV file: {0}", filepath.string());
		}
	}
}
