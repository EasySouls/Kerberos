#pragma once

#include <filesystem>

namespace Kerberos
{
	class AssetsPanel
	{
	public:
		AssetsPanel();
		~AssetsPanel() = default;
		
		void OnImGuiRender();

	private:
		std::filesystem::path m_CurrentDirectory = std::filesystem::current_path();
	};

}

