#pragma once

#include "Kerberos/Core.h"
#include "Kerberos/Assets/Asset.h"

/**
 * Represents a sound asset and provides playback and volume controls.
 */
 
/**
 * Construct a Sound with the given name.
 * @param name Human-readable name of the sound asset.
 */

/**
 * Begin playback of the sound.
 */

/**
 * Stop playback of the sound.
 */

/**
 * Increase the current volume by `delta`.
 * @param delta Amount to increase the volume by (additive).
 */

/**
 * Decrease the current volume by `delta`.
 * @param delta Amount to decrease the volume by (subtractive).
 */

/**
 * Set the current volume to `volume`.
 * @param volume New volume level.
 */

/**
 * Restore the volume to its default value.
 */

/**
 * Mute the sound.
 */

/**
 * Get the name of the sound asset.
 * @returns Reference to the internal name string.
 */

/**
 * Get the asset type for this asset.
 * @returns The value `AssetType::Sound`.
 */

/**
 * Get the UUID that identifies this sound in the Audio Manager.
 * @returns UUID of the sound asset.
 */
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

		void IncreaseVolume(float delta) const;
		void DecreaseVolume(float delta) const;
		void SetVolume(float volume) const;
		void ResetVolume() const;
		void Mute() const;

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