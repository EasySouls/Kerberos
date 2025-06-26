#pragma once

#include "Kerberos/Core.h"
#include "Kerberos/Renderer/Texture.h"
#include "Kerberos/Project/Project.h"

#include <filesystem>
#include <map>
#include <set>

#include "imgui/imgui.h"


namespace Kerberos
{
	class AssetsPanel
	{
	public:
		AssetsPanel();
		~AssetsPanel() = default;

		/**
		* Updates the current directory to the specified path.
		*/
		void SetCurrentDir(const std::filesystem::path& path);

		void OnImGuiRender();

	private:
		/**
		* Context menu displayed when right-clicking on a file in the Assets panel.
		*/
		void ShowFileContextMenu(std::filesystem::path::iterator::reference path);

		/**
		* Context menu displayed when right-clicking on a folder in the Assets panel.
		*/ 
		void ShowFolderContextMenu(std::filesystem::path::iterator::reference path);

		/**
		* Context menu displayed when right-clicking on an empty area of the Assets panel.
		*/
		void ShowContextMenu(ImGuiPopupFlags popupFlags) const;

	private:
		/**
		 * 
		 */
		void RefreshAssetTree();

	private:
		std::filesystem::path m_AssetsDirectory = "Assets";
		std::filesystem::path m_CurrentDirectory = std::filesystem::current_path();
		Ref<Texture2D> m_FolderIcon;
		Ref<Texture2D> m_FileIcon;

		std::map<std::filesystem::path, Ref<Texture2D>> m_AssetImages;

		struct TreeNode
		{
			std::filesystem::path Path;

			TreeNode* Parent = nullptr;
			std::set<TreeNode*> Children;

			TreeNode(std::filesystem::path path, TreeNode* parent = nullptr)
				: Path(std::move(path)), Parent(parent)
			{
			}
		};


		/**
		 * The root node of the asset tree.
		 * Relative to the asset directory.
		 */
		TreeNode m_RootNode;

		std::map<std::filesystem::path, std::vector<std::filesystem::path>> m_AssetTree;

		enum class Mode : uint8_t
		{
			Asset = 0,
			Filesystem = 1
		};

		Mode m_Mode = Mode::Asset;
	};

}

