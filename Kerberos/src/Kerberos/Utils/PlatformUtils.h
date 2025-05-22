#pragma once

namespace Kerberos
{
	class FileDialog
	{
	public:
		[[nodiscard]]
		static std::string OpenFile(const char* filter = "");

		[[nodiscard]]
		static std::string SaveFile(const char* filter = "");
	};
}