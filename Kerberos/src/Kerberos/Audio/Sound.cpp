#include "kbrpch.h"
#include "Sound.h"

#include "Kerberos/Application.h"

namespace Kerberos
{
	/**
	 * @brief Starts playback of this sound.
	 *
	 * Begins playing the audio resource associated with this Sound instance via the global audio manager.
	 */
	void Sound::Play() const 
	{
		Application::Get().GetAudioManager()->Play(m_SoundID);
	}

	/**
	 * @brief Stops playback of this sound.
	 *
	 * Stops playback of the sound identified by this instance's sound ID via the application's AudioManager.
	 * If the sound is not currently playing, the call has no effect.
	 */
	void Sound::Stop() const 
	{
		Application::Get().GetAudioManager()->Stop(m_SoundID);
	}

	/**
	 * @brief Increases this sound's playback volume by the specified amount.
	 *
	 * @param delta Amount to increase the current volume by. Positive values raise the volume. 
	 */
	void Sound::IncreaseVolume(const float delta) const
	{
		Application::Get().GetAudioManager()->IncreaseVolume(m_SoundID, delta);
	}

	/**
	 * @brief Decreases this sound's playback volume by the specified amount.
	 *
	 * @param delta Amount to reduce the current volume by. */
	void Sound::DecreaseVolume(const float delta) const
	{
		Application::Get().GetAudioManager()->DecreaseVolume(m_SoundID, delta);
	}

	/**
	 * @brief Set the playback volume for this sound.
	 *
	 * @param volume Desired volume level; higher values increase loudness for this sound.
	 */
	void Sound::SetVolume(const float volume) const
	{
		Application::Get().GetAudioManager()->SetVolume(m_SoundID, volume);
	}

	/**
	 * @brief Reset the volume of this sound to its default level.
	 *
	 * Invokes the global audio manager to restore this sound's volume to the manager-defined default.
	 */
	void Sound::ResetVolume() const
	{
		Application::Get().GetAudioManager()->ResetVolume(m_SoundID);
	}

	/**
	 * @brief Mutes this sound so it produces no audible output.
	 *
	 * After calling this method, the sound remains muted until its volume is changed or it is explicitly unmuted.
	 */
	void Sound::Mute() const
	{
		Application::Get().GetAudioManager()->Mute(m_SoundID);
	}
}