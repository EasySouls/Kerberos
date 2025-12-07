#pragma once

#include "Kerberos/Core.h"
#include "Kerberos/Renderer/Texture.h"
#include "Kerberos/Project/Project.h"
#include "../Notification/NotificationManager.h"
#include <filesystem>
#include <map>
#include <set>

#include "imgui/imgui.h"


/**
 * Set the current directory displayed by the assets panel.
 * @param path Filesystem path to use as the current directory.
 */

/**
 * Display a context menu for a file entry in the assets panel.
 * @param path Iterator reference to the file's path within the current directory listing.
 */

/**
 * Display a context menu for a folder entry in the assets panel.
 * @param path Iterator reference to the folder's path within the current directory listing.
 */

/**
 * Display a context menu for an empty area of the assets panel using the provided ImGui popup flags.
 * @param popupFlags ImGui popup flags to control menu behavior when opened.
 */

/**
 * Handle a drag-and-drop operation for an asset onto a target filename.
 * @param handle Asset handle being dragged.
 * @param filename Target filename or path the asset was dropped onto.
 */
namespace Kerberos
{
	class AssetsPanel
	{
	public:
		explicit AssetsPanel(NotificationManager notificationManager);
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

		void RefreshAssetTree();

		void ImportAssetDialog();

		void HandleAssetDragAndDrop(AssetHandle handle, const std::filesystem::path& filename);

	private:
		std::filesystem::path m_AssetsDirectory = "Assets";
		std::filesystem::path m_CurrentDirectory = std::filesystem::current_path();
		Ref<Texture2D> m_FolderIcon;
		Ref<Texture2D> m_FileIcon;

		std::map<std::filesystem::path, Ref<Texture2D>> m_AssetImages;

		struct TreeNode
		{
			std::filesystem::path Path;
			AssetHandle Handle = AssetHandle::Invalid();

			uint32_t Parent = static_cast<uint32_t>(-1);
			std::map<std::filesystem::path, uint32_t> Children;

			explicit TreeNode(std::filesystem::path path, const AssetHandle handle)
				: Path(std::move(path)), Handle(handle)
			{
			}
		};

		std::vector<TreeNode> m_AssetTreeNodes;

		enum class Mode : uint8_t
		{
			Asset = 0,
			Filesystem = 1
		};

		Mode m_Mode = Mode::Filesystem;

		NotificationManager m_NotificationManager;
	};

}
