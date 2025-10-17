#pragma once

#include "Kerberos/Core.h"

namespace Kerberos 
{
	class Sound
	{
	public:
		virtual ~Sound() = default;
		virtual void Play() = 0;
		virtual void Stop() = 0;
		virtual void SetVolume(float volume) = 0;
		virtual float GetVolume() const = 0;
		virtual bool IsPlaying() const = 0;
	};

}