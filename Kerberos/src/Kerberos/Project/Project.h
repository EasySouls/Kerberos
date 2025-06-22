#pragma once

#include "Kerberos/Core.h"

#include <filesystem>

namespace Kerberos
{
	struct ProjectInfo
	{
		std::string Name = "Untitled";
		std::filesystem::path AssetDirectory;

		std::filesystem::path StartScenePath;
	};

	class Project
	{
	public:
		static Ref<Project> New();
		static Ref<Project> Load(const std::filesystem::path& filepath);
		static bool SaveActive(const std::filesystem::path& filepath);

		static const std::filesystem::path& GetAssetDirectory()
		{
			KBR_CORE_ASSERT(s_ActiveProject, "An active project is not set!");

			return s_ActiveProject->m_Info.AssetDirectory;
		}

		static ProjectInfo& GetInfo()
		{
			KBR_CORE_ASSERT(s_ActiveProject, "An active project is not set!");

			return s_ActiveProject->m_Info;
		}

	private:
		ProjectInfo m_Info;

		inline static Ref<Project> s_ActiveProject;
	};
}