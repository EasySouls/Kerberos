#include "AssetsPanel.h"

#include <imgui/imgui.h>
#include <Kerberos/Utils/PlatformUtils.h>

#include <algorithm>

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
		static float thumbnailSize = 128.0f;
		static float cellSize = thumbnailSize + padding;

		const float panelWidth = ImGui::GetContentRegionAvail().x;
		int columns = static_cast<int>(panelWidth / cellSize);
		columns = std::max(columns, 1);

		ImGui::Columns(columns, nullptr, false);

		for (const auto& entry : std::filesystem::directory_iterator(m_CurrentDirectory))
		{
			const std::filesystem::path& path = entry.path();
			const auto& relativePath = std::filesystem::relative(path, ASSETS_DIRECTORY);
			const std::string fileName = relativePath.filename().string();

			if (entry.is_directory())
			{
				ImGui::ImageButton(path.string().c_str(), m_FolderIcon->GetRendererID(), {64, 64}, {0, 1}, {1, 0});
				if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
				{
					m_CurrentDirectory /= path.filename();
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

					ImGui::ImageButton(path.string().c_str(), m_AssetImages[path]->GetRendererID(), {64, 64}, {0, 1}, {1, 0});
					if (ImGui::IsItemHovered())
					{
						ImGui::BeginTooltip();
						ImGui::Text("%s", fileName.c_str());
						ImGui::EndTooltip();
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
				}
				else
				{
					ImGui::ImageButton(path.string().c_str(), m_FileIcon->GetRendererID(), {64, 64}, {0, 1}, {1, 0});
					if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
					{
						
					}
				}

			}
			ImGui::TextWrapped(fileName.c_str());

			ImGui::NextColumn();
		}

		ImGui::Columns(1);

		ImGui::End();
	}
}
