#include "AssetsPanel.h"

#include <imgui/imgui.h>
#include <Kerberos/Utils/PlatformUtils.h>

#include <algorithm>

#include "Kerberos/Log.h"
#include "Kerberos/Assets/Importers/TextureImporter.h"
#include "Kerberos/Project/Project.h"

namespace Kerberos
{
	AssetsPanel::AssetsPanel()
		: m_AssetsDirectory(Project::GetAssetDirectory()), m_CurrentDirectory(m_AssetsDirectory), m_RootNode("/")
	{
		m_FolderIcon = TextureImporter::ImportTexture("Assets/Editor/directory_icon.png");
		m_FileIcon = TextureImporter::ImportTexture("Assets/Editor/file_icon.png");

		RefreshAssetTree();
	}

	void AssetsPanel::OnImGuiRender()
	{
		ImGui::Begin("Assets");

		const auto& relativeDir = std::filesystem::relative(m_CurrentDirectory, m_AssetsDirectory);
		const std::string title = relativeDir.string() == "." ? "Assets" : "Assets" + std::string(1, std::filesystem::path::preferred_separator) + relativeDir.string();
		ImGui::Text("Current Directory: %s", title.data());

		if (m_CurrentDirectory != m_AssetsDirectory)
		{
			if (ImGui::Button("Back"))
			{
				m_CurrentDirectory = m_CurrentDirectory.parent_path();
			}
		}

		static float padding = 10.0f;
		static float thumbnailSize = 96.0f;
		static float cellSize = thumbnailSize + padding;

		const float panelWidth = ImGui::GetContentRegionAvail().x;
		int columns = static_cast<int>(panelWidth / cellSize);
		columns = std::max(columns, 1);

		/// Show default context menu when right-clicking on an empty space in the panel
		ShowContextMenu(ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems);

		ImGui::Columns(columns, nullptr, false);

		if (m_Mode == Mode::Asset)
		{

		}
		else
		{

			for (const auto& entry : std::filesystem::directory_iterator(m_CurrentDirectory))
			{
				const std::filesystem::path& path = entry.path();
				const auto& relativePath = std::filesystem::relative(path, m_AssetsDirectory);
				const std::string fileName = relativePath.filename().string();

				ImGui::PushID(path.string().c_str());

				ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.2f, 0.2f, 0.5f));

				if (entry.is_directory())
				{
					ImGui::ImageButton(path.string().c_str(), m_FolderIcon->GetRendererID(), { thumbnailSize, thumbnailSize }, { 0, 1 }, { 1, 0 });
					if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
					{
						m_CurrentDirectory /= path.filename();
					}

					ShowFolderContextMenu(path);
				}
				else
				{
					const auto fileExtension = path.extension();
					const bool isImageFile = (fileExtension == ".png" || fileExtension == ".jpg" || fileExtension == ".jpeg");
					if (isImageFile)
					{
						if (!m_AssetImages.contains(path))
						{
							/// Load the image and store it in the map
							const std::string fullPath = path.string();
							m_AssetImages[path] = Texture2D::Create(fullPath);
						}

						ImGui::ImageButton(path.string().c_str(), m_AssetImages[path]->GetRendererID(), { thumbnailSize, thumbnailSize }, { 0, 1 }, { 1, 0 });
						if (ImGui::IsItemHovered())
						{
							ImGui::BeginTooltip();
							ImGui::Text("%s", fileName.c_str());
							ImGui::EndTooltip();
						}
					}
					else
					{
						ImGui::ImageButton(path.string().c_str(), m_FileIcon->GetRendererID(), { thumbnailSize, thumbnailSize }, { 0, 1 }, { 1, 0 });
					}

					ShowFileContextMenu(path);

					if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
					{
						const auto itemPath = relativePath.string();

						ImGui::SetDragDropPayload("ASSET_BROWSER_ITEM", itemPath.c_str(), itemPath.size() + 1, ImGuiCond_Once);
						ImGui::Text("%s", fileName.c_str());
						ImGui::EndDragDropSource();
					}

					/// Open the file on double click
					if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
					{
						/// TODO: If the file is a kerberos scene, open it in the scene editor

						/// Open the file using the default application
						const bool opened = FileOperations::OpenFile(path.string().c_str());
						if (!opened)
						{
							ImGui::Text("Could not open file: %s", fileName.c_str());
						}
					}

				}
				ImGui::TextWrapped("%s", fileName.c_str());

				ImGui::NextColumn();

				ImGui::PopStyleColor();

				ImGui::PopID();
			}
		}


		ImGui::Columns(1);

		ImGui::End();
	}

	void AssetsPanel::ShowFileContextMenu(std::filesystem::path::iterator::reference path)
	{
		if (ImGui::BeginPopupContextItem("FileContext")) // "FileContext" is the ID for this popup
		{
			ImGui::TextDisabled("%s", path.string().c_str());
			ImGui::Separator();
			if (ImGui::MenuItem("Open"))
			{
				const bool success = FileOperations::OpenFile(path.string().c_str());
				if (!success)
				{
					ImGui::Text("Could not open file: %s", path.filename().string().c_str());
				}
				ImGui::CloseCurrentPopup();
			}
			if (ImGui::MenuItem("Delete File"))
			{
				// TODO: Add confirmation!
				if (m_AssetImages.contains(path))
				{
					m_AssetImages.erase(path); // Release texture if it was loaded
				}
				std::filesystem::remove(path);
				ImGui::CloseCurrentPopup();
			}
			if (ImGui::MenuItem("Import as asset"))
			{
				const Ref<EditorAssetManager> assetManager = Project::GetActive()->GetEditorAssetManager();
				assetManager->ImportAsset(path);

				ImGui::CloseCurrentPopup();
			}
			// Add other file-specific menu items (e.g., Rename, Show in Explorer)
			ImGui::EndPopup();
		}
	}

	void AssetsPanel::ShowFolderContextMenu(std::filesystem::path::iterator::reference path)
	{
		if (ImGui::BeginPopupContextItem("FolderContext")) // "FolderContext" is the ID for this popup
		{
			ImGui::TextDisabled("%s", path.string().c_str());
			ImGui::Separator();
			if (ImGui::MenuItem("Open"))
			{
				m_CurrentDirectory /= path.filename();
				ImGui::CloseCurrentPopup();
			}
			if (ImGui::MenuItem("Delete Folder"))
			{
				// TODO: Add confirmation dialog!
				std::filesystem::remove_all(path); // Use remove_all for directories
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}
	}

	void AssetsPanel::ShowContextMenu(const ImGuiPopupFlags popupFlags) const
	{
		if (ImGui::BeginPopupContextWindow(nullptr, popupFlags))
		{
			if (ImGui::MenuItem("New Folder"))
			{
				std::filesystem::path newFolderPath = m_CurrentDirectory / "New Folder";
				int counter = 1;
				while (std::filesystem::exists(newFolderPath))
				{
					newFolderPath = m_CurrentDirectory / ("New Folder " + std::to_string(counter++));
				}
				std::filesystem::create_directory(newFolderPath);
			}

			ImGui::EndPopup();
		}
	}

	void AssetsPanel::RefreshAssetTree()
	{
		const AssetRegistry& assetRegistry = Project::GetActive()->GetEditorAssetManager()->GetAssetRegistry();
		for (const auto& [handle, metadata] : assetRegistry)
		{
			
		}
	}

	void AssetsPanel::SetCurrentDir(const std::filesystem::path& path)
	{
		if (std::filesystem::exists(path) && std::filesystem::is_directory(path))
		{
			m_CurrentDirectory = path;
			return;
		}

		KBR_CORE_ERROR("Invalid directory path: {0}", path.string());
	}

}
