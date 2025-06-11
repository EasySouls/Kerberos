#include "AssetsPanel.h"

#include <imgui/imgui.h>

namespace Kerberos
{
	static const std::filesystem::path ASSETS_DIRECTORY = "Assets";

	AssetsPanel::AssetsPanel()
		: m_CurrentDirectory(ASSETS_DIRECTORY)
	{

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

		for (const auto& entry : std::filesystem::directory_iterator(m_CurrentDirectory))
		{
			const std::filesystem::path& path = entry.path();
			const auto& relativePath = std::filesystem::relative(path, ASSETS_DIRECTORY);
			const std::string fileName = relativePath.filename().string();

			if (entry.is_directory())
			{
				if (ImGui::Selectable(fileName.c_str()))
				{
					m_CurrentDirectory /= path.filename();
				}
			}
			else
			{
				if (ImGui::Selectable(fileName.c_str()))
				{
				}
			}

		}

		ImGui::End();
	}
}
