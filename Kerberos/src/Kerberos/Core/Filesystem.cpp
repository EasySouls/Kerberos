#include "kbrpch.h"
#include "Filesystem.h"

namespace Kerberos
{
	std::string Filesystem::ReadTextFile(const std::filesystem::path& filepath) 
	{
		std::ifstream stream(filepath);
		if (!stream)
		{
			return "";
		}

		std::string content((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
		return content;
	}

	char* Filesystem::ReadBytes(const std::filesystem::path& filepath, uint32_t* outSize)
	{
		std::ifstream stream(filepath, std::ios::binary | std::ios::ate);

		if (!stream)
		{
			return nullptr;
		}

		const std::streampos end = stream.tellg();
		stream.seekg(0, std::ios::beg);
		const std::streamoff size = end - stream.tellg();

		if (size == 0)
		{
			return nullptr;
		}

		char* buffer = new char[size];
		stream.read(buffer, size);
		stream.close();

		*outSize = static_cast<uint32_t>(size);
		return buffer;
	}
}