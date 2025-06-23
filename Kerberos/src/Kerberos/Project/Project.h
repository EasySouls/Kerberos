#pragma once

#include "Kerberos/Core.h"
#include "Kerberos/Assets/AssetManagerBase.h"

#include <filesystem>

namespace Kerberos
{
	struct ProjectInfo
	{
		std::string Name = "Untitled";
		std::filesystem::path AssetDirectory = "Assets";

		std::filesystem::path StartScenePath;
	};

	class Project
	{
	public:
		static Ref<Project> New();
		static Ref<Project> Load(const std::filesystem::path& filepath);
		static bool SaveActive(const std::filesystem::path& filepath);

		/**
		* Returns the path to the active project's asset directory.
		*/
		static const std::filesystem::path& GetAssetDirectory()
		{
			KBR_CORE_ASSERT(s_ActiveProject, "An active project is not set!");

			return s_ActiveProject->m_Info.AssetDirectory;
		}

		static const std::filesystem::path& GetProjectDirectory()
		{
			KBR_CORE_ASSERT(s_ActiveProject, "An active project is not set!");

			return s_ActiveProject->m_ProjectDirectory;
		}

		static std::filesystem::path GetAssetFileSystemPath(const std::filesystem::path& assetPath)
		{
			KBR_CORE_ASSERT(s_ActiveProject, "An active project is not set!");

			return GetProjectDirectory() / GetAssetDirectory() / assetPath;
		}

		ProjectInfo& GetInfo() { return m_Info; }

		static Ref<Project> GetActive() { return s_ActiveProject; }

		Ref<AssetManagerBase> GetAssetManager() { return m_AssetManager; }

	private:
		ProjectInfo m_Info;
		std::filesystem::path m_ProjectDirectory;

		Ref<AssetManagerBase> m_AssetManager;

		inline static Ref<Project> s_ActiveProject;
	};
}