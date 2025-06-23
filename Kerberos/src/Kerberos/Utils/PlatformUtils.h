#pragma once

namespace Kerberos
{
	class FileDialog
	{
	public:
		/**
		* Opens a file dialog to select a file to open.
		* @param filter A string that specifies the file types to display in the dialog. The format is "Description (*.ext1;*.ext2)|*.ext1;*.ext2".
		* @return an absolute path to a file selected by the user, or an empty string if the user cancelled the dialog.
		*/
		[[nodiscard]]
		static std::string OpenFile(const char* filter = "");


		/**
		 * Opens a file dialog to select a file to save.
		 * @param filter A string that specifies the file types to display in the dialog. The format is "Description (*.ext1;*.ext2)|*.ext1;*.ext2".
		 * @return an absolute path to a file selected by the user, or an empty string if the user cancelled the dialog.
		 */
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
		static bool OpenFile(const char* path);
	};
}