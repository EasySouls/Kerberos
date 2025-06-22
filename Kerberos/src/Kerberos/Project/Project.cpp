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
		const Ref<Project> loadedProject = CreateRef<Project>();

		const ProjectSerializer deserializer(s_ActiveProject);
		if (deserializer.Deserialize(filepath))
		{
			s_ActiveProject = loadedProject;
			return s_ActiveProject;
		}

		return nullptr;
	}

	bool Project::SaveActive(const std::filesystem::path& filepath) 
	{
		const ProjectSerializer serializer(s_ActiveProject);
		return serializer.Serialize(filepath);
	}
}
