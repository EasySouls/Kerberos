#pragma once 

#include "Kerberos/Core.h"
#include "Texture.h"

#include <filesystem>
#include <string>

namespace Kerberos 
{
	struct MSDFData;

	class Font
	{
	public:
		explicit Font(std::string name, const std::filesystem::path& filepath);
		~Font();

		Font(const Font& other) = default;
		Font(Font&& other) noexcept = default;
		Font& operator=(const Font& other) = default;
		Font& operator=(Font&& other) noexcept = default;

		Ref<Texture2D> GetAtlasTexture() const { return m_AtlasTexture; }

	private:
		std::string m_Name;
		MSDFData* m_MSDFData = nullptr;
		Ref<Texture2D> m_AtlasTexture;
	};
}