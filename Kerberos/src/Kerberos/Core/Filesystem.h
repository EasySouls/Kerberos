#pragma once

#include <filesystem>

namespace Kerberos
{
	class Filesystem
	{
	public:
		static std::string ReadTextFile(const std::filesystem::path& filepath);
		static char* ReadBytes(const std::filesystem::path& filepath, uint32_t* outSize);
	};
}