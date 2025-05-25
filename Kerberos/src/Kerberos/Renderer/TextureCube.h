#pragma once
#include "Texture.h"

namespace Kerberos
{
	class TextureCube : public Texture
	{
	public:
		~TextureCube() override = default;

		virtual const std::string& GetName() const = 0;

		static Ref<TextureCube> Create(const std::string& name, const std::vector<std::string>& faces, bool generateMipmaps = true, bool srgb = false);
	};
}
