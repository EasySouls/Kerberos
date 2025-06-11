#include "AssetsPanel.h"

#include <imgui/imgui.h>
#include <Kerberos/Utils/PlatformUtils.h>

#include <algorithm>

#include "Kerberos/Log.h"

namespace Kerberos
{
	static const std::filesystem::path ASSETS_DIRECTORY = "Assets";

	AssetsPanel::AssetsPanel()
		: m_CurrentDirectory(ASSETS_DIRECTORY)
	{
		m_FolderIcon = Texture2D::Create("Assets/Editor/directory_icon.png");
		m_FileIcon = Texture2D::Create("Assets/Editor/file_icon.png");
	}

	void AssetsPanel::OnImGuiRender()
	{
		ImGui::Begin("Assets");

		const auto& relativeDir = std::filesystem::relative(m_CurrentDirectory, ASSETS_DIRECTORY);
		const std::string title = relativeDir.string() == "." ? "Assets" : "Assets\\" + relativeDir.string();
		ImGui::Text("Current Directory: %s", title.data());

		if (m_CurrentDirectory != ASSETS_DIRECTORY)
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
		ShowContextMenu(1 | ImGuiPopupFlags_NoOpenOverItems);

		ImGui::Columns(columns, nullptr, false);

		for (const auto& entry : std::filesystem::directory_iterator(m_CurrentDirectory))
		{
			const std::filesystem::path& path = entry.path();
			const auto& relativePath = std::filesystem::relative(path, ASSETS_DIRECTORY);
			const std::string fileName = relativePath.filename().string();

			if (entry.is_directory())
			{
				ImGui::ImageButton(path.string().c_str(), m_FolderIcon->GetRendererID(), { thumbnailSize, thumbnailSize }, { 0, 1 }, { 1, 0 });
				if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
				{
					m_CurrentDirectory /= path.filename();
				}
				if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right))
				{
					ShowFolderContextMenu(path);
				}
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

					ImGui::ImageButton(path.string().c_str(), m_AssetImages[path]->GetRendererID(), { 64, 64 }, { 0, 1 }, { 1, 0 });
					if (ImGui::IsItemHovered())
					{
						ImGui::BeginTooltip();
						ImGui::Text("%s", fileName.c_str());
						ImGui::EndTooltip();
					}
				}
				else
				{
					ImGui::ImageButton(path.string().c_str(), m_FileIcon->GetRendererID(), { 64, 64 }, { 0, 1 }, { 1, 0 });
					if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
					{

					}
				}

				/// Open the file on double click
				if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
				{
					/// Open the file using the default application
					const bool opened = FileOperations::OpenFile(path.string().c_str());
					if (!opened)
					{
						ImGui::Text("Could not open file: %s", fileName.c_str());
					}
				}
				/// Show context menu on left click
				if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right))
				{
					ShowFolderContextMenu(path);
				}

			}
			ImGui::TextWrapped(fileName.c_str());

			ImGui::NextColumn();
		}

		ImGui::Columns(1);

		ImGui::End();
	}

	void AssetsPanel::ShowFileContextMenu(std::filesystem::path::iterator::reference path) const 
	{
		if (ImGui::BeginPopup("File Context Menu"))
		{
			if (ImGui::MenuItem("Delete"))
			{
				std::filesystem::remove(path);
			}
			ImGui::EndPopup();
		}
	}

	void AssetsPanel::ShowFolderContextMenu(std::filesystem::path::iterator::reference path) const 
	{
		ImGui::OpenPopup("Folder Context Menu");

		if (ImGui::BeginPopup("Folder Context Menu"))
		{
			if (ImGui::MenuItem("Delete"))
			{
				std::filesystem::remove(path);
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
