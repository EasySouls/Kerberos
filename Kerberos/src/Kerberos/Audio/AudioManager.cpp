#include "kbrpch.h"
#include "AudioManager.h"


#ifdef KBR_PLATFORM_WINDOWS
#include "Platform/Windows/Audio/XAudio2AudioManager.h"
#endif

namespace Kerberos 
{
	/**
 * @brief Creates a platform-specific AudioManager instance.
 *
 * On Windows builds this returns a new XAudio2AudioManager. On unsupported platforms
 * the function triggers a core assertion and returns `nullptr`.
 *
 * @return AudioManager* Pointer to a heap-allocated AudioManager instance appropriate
 * for the current platform, or `nullptr` if no implementation exists. Caller takes ownership.
 */
AudioManager* AudioManager::Create() 
	{
	#ifdef KBR_PLATFORM_WINDOWS
		return new XAudio2AudioManager();
	#endif

	KBR_CORE_ASSERT(false, "No AudioManager implementation for this platform!");
	return nullptr;
}
}