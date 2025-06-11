#pragma once

#include "Kerberos/Core.h"
#include "Kerberos/Renderer/Texture.h"
#include <filesystem>
#include <map>


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

		std::map<std::filesystem::path, Ref<Texture2D>> m_AssetImages;
	};

}

