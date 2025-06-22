#pragma once

#include "Project.h"

namespace Kerberos
{
	class ProjectSerializer
	{
	public:
		explicit ProjectSerializer(const Ref<Project>& project);

		bool Serialize(const std::filesystem::path& filepath) const;
		bool Deserialize(const std::filesystem::path& filepath) const;

	private:
		Ref<Project> m_Project;
	};
}