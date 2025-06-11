#pragma once

#include "Kerberos/Core.h"
#include "Kerberos/Renderer/Texture.h"
#include <filesystem>
#include <map>

#include "imgui/imgui.h"


namespace Kerberos
{
	class AssetsPanel
	{
	public:
		AssetsPanel();
		~AssetsPanel() = default;

		void SetCurrentDir(const std::filesystem::path& path);

		void OnImGuiRender();

	private:
		/**
		* Context menu displayed when right-clicking on a file in the Assets panel.
		*/
		void ShowFileContextMenu(std::filesystem::path::iterator::reference path) const;

		/**
		* Context menu displayed when right-clicking on a folder in the Assets panel.
		*/ 
		void ShowFolderContextMenu(std::filesystem::path::iterator::reference path) const;

		/**
		* Context menu displayed when right-clicking on an empty area of the Assets panel.
		*/
		void ShowContextMenu(ImGuiPopupFlags popupFlags) const;

	private:
		std::filesystem::path m_CurrentDirectory = std::filesystem::current_path();
		Ref<Texture2D> m_FolderIcon;
		Ref<Texture2D> m_FileIcon;

		std::map<std::filesystem::path, Ref<Texture2D>> m_AssetImages;
	};

}

