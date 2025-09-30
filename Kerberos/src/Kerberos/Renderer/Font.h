#pragma once 

#include "Kerberos/Core.h"
#include "Texture.h"

#include <filesystem>
#include <string>

namespace Kerberos 
{
	struct MSDFData;

	struct FontMetrics
	{
		float Ascender = 0.0f;
		float Descender = 0.0f;
		float LineHeight = 0.0f;
	};;

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
		FontMetrics GetMetrics() const;
		const std::string& GetName() const { return m_Name; }

		bool HasCharacter(char c) const;
		void GetQuadAtlasBounds(char character, double& al, double& ab, double& ar, double& at) const;
		void GetQuadPlaneBounds(char character, double& pl, double& pb, double& pr, double& pt) const;
		double GetAdvance(char character) const;
		void GetNextAdvance(double& advance, char character, char nextCharacter) const;

		static Ref<Font> GetDefaultFont();

	private:
		std::string m_Name;
		MSDFData* m_MSDFData = nullptr;
		Ref<Texture2D> m_AtlasTexture;
	};
}