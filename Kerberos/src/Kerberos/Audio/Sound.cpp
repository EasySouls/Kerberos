#include "kbrpch.h"
#include "Sound.h"

#include "Kerberos/Application.h"

namespace Kerberos
{
	void Sound::Play() const 
	{
		Application::Get().GetAudioManager()->Play(m_SoundID);
	}

	void Sound::Stop() const 
	{
		Application::Get().GetAudioManager()->Stop(m_SoundID);
	}
}
