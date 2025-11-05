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

		void Play() const;
		void Stop() const;

		const std::string& GetName() const { return m_Name; }
		AssetType GetType() override { return AssetType::Sound; }
		UUID GetSoundID() const { return m_SoundID; }

	private:
		std::string m_Name;

		/**
		* The UUID of the sound asset in the Audio Manager.
		*/
		UUID m_SoundID;
	};
}
