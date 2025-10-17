#include "kbrpch.h"
#include "AudioManager.h"


#ifdef KBR_PLATFORM_WINDOWS
#include "Platform/Windows/Audio/XAudio2AudioManager.h"
#endif

namespace Kerberos 
{
	AudioManager* AudioManager::Create() 
	{
	#ifdef KBR_PLATFORM_WINDOWS
		return new XAudio2AudioManager();
	#endif

	KBR_CORE_ASSERT(false, "No AudioManager implementation for this platform!");
	return nullptr;
}
}
