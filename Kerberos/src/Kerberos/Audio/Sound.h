#pragma once

#include "Kerberos/Core.h"
#include "Kerberos/Assets/Asset.h"

namespace Kerberos 
{
	class Sound : public Asset
	{
	public:
		explicit Sound(std::string name)
			: m_Name(std::move(name))
		{}

		~Sound() override = default;

		Sound(const Sound& other) = default;
		Sound(Sound&& other) noexcept = default;
		Sound& operator=(const Sound& other) = default;
		Sound& operator=(Sound&& other) noexcept = default;

		virtual void Play() = 0;
		virtual void Stop() = 0;
		virtual void SetVolume(float volume) = 0;
		virtual float GetVolume() const = 0;
		virtual bool IsPlaying() const = 0;

		const std::string& GetName() const { return m_Name; }
		AssetType GetType() override { return AssetType::Sound; }

	private:
		std::string m_Name;
	};
}