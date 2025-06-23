#include "kbrpch.h"
#include "Project.h"
#include "ProjectSerializer.h"

namespace Kerberos
{
	Ref<Project> Project::New() 
	{
		s_ActiveProject = CreateRef<Project>();

		return s_ActiveProject;
	}

	Ref<Project> Project::Load(const std::filesystem::path& filepath) 
	{
		const Ref<Project> projectToLoad = CreateRef<Project>();

		const ProjectSerializer deserializer(projectToLoad);
		if (deserializer.Deserialize(filepath))
		{
			projectToLoad->m_ProjectDirectory = filepath.parent_path();
			s_ActiveProject = projectToLoad;
			KBR_CORE_INFO("Project is loaded from {}", std::filesystem::absolute(filepath).string());
			return s_ActiveProject;
		}

		return nullptr;
	}

	bool Project::SaveActive(const std::filesystem::path& filepath) 
	{
		const ProjectSerializer serializer(s_ActiveProject);
		if (serializer.Serialize(filepath))
		{
			s_ActiveProject->m_ProjectDirectory = filepath.parent_path();
			KBR_CORE_INFO("Project is saved to {}", std::filesystem::absolute(filepath).string());
			return true;
		}
		else
		{
			KBR_CORE_WARN("Could not save project to {}", std::filesystem::absolute(filepath).string());
			return false;
		}
	}
}
