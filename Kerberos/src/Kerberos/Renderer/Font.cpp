#include "kbrpch.h"
#include "Font.h"

#include <algorithm>


#undef INFINITE
#include <msdfgen.h>
#include "ext/import-font.h"
#include "ext/save-png.h"

namespace Kerberos 
{
	Font::Font(const std::filesystem::path& filepath) 
	{
		if (!std::filesystem::exists(filepath))
		{
			KBR_CORE_ASSERT(false, "Font file does not exist: {0}", filepath.string());
			return;
		}
		
		msdfgen::FreetypeHandle* ft = msdfgen::initializeFreetype();
		if (!ft) 
		{
			KBR_CORE_ASSERT(false, "Could not initialize FreeType library!");
			return;
		}

		const std::string filepathStr = filepath.string();
		msdfgen::FontHandle* font = msdfgen::loadFont(ft, filepathStr.c_str());
		if (!font) 
		{
			KBR_CORE_ASSERT(false, "Could not load font: {0}", filepath.string());
			msdfgen::deinitializeFreetype(ft);
			return;
		}

		//msdfgen::Shape shape;
		//if (msdfgen::loadGlyph(shape, font, 'C'))
		//{
		//	shape.normalize();
		//	//                      max. angle
		//	msdfgen::edgeColoringSimple(shape, 3.0);
		//	//           image width, height
		//	msdfgen::Bitmap<float, 3> msdf(32, 32);
		//	//                     range, scale, translation
		//	msdfgen::generateMSDF(msdf, shape, 4.0, 1.0, msdfgen::Vector2(4.0, 4.0));
		//	msdfgen::savePng(msdf, "output.png");
		//}
		//else 
		//{
		//	KBR_CORE_ASSERT(false, "Could not load glyph for character 'C' from font: {0}", filepath.string());
		//}
		msdfgen::destroyFont(font);
		msdfgen::deinitializeFreetype(ft);
	}
}
