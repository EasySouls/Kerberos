#pragma once 

#include <filesystem>

namespace Kerberos 
{
	class Font
	{
	public:
		explicit Font(const std::filesystem::path& filepath);
	};
}