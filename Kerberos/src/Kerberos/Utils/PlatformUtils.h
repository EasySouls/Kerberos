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

	class FileOperations
	{
	public:
		/**
		* Opens the give file with the default application associated with the file type.
		* @param path The path to the file to open.
		* @return whether the file was successfully opened or not.
		*/
		[[nodiscard]]
		static bool OpenFile(const char* path = nullptr);
	};
}