#include "kbrpch.h"
#include "SceneSerializer.h"

#include "Kerberos/Scene/Entity.h"
#include "Kerberos/Scene/Components.h"

#include <yaml-cpp/yaml.h>

#include <fstream>
#include <filesystem>

namespace Kerberos
{
	static void SerializeEntity(YAML::Emitter& out, Entity entity)
	{
		out << YAML::BeginMap;
		out << YAML::Key << "Entity" << YAML::Value << static_cast<uint32_t>(entity);

		if (entity.HasComponent<TagComponent>())
		{
			out << YAML::Key << "TagComponent";
			out << YAML::BeginMap;
			const auto& tag = entity.GetComponent<TagComponent>().Tag;
			out << YAML::Key << "Tag" << YAML::Value << tag;
			out << YAML::EndMap;
		}

		//if (entity.HasComponent<TransformComponent>())
		//{
		//	out << YAML::Key << "TransformComponent";
		//	out << YAML::BeginMap;
		//	const auto& transform = entity.GetComponent<TransformComponent>();
		//	out << YAML::Key << "Translation" << YAML::Value << transform.Translation;
		//	out << YAML::Key << "Rotation" << YAML::Value << transform.Rotation;
		//	out << YAML::Key << "Scale" << YAML::Value << transform.Scale;
		//	out << YAML::EndMap;
		//}

		out << YAML::EndMap;

	}

	void SceneSerializer::Serialize(const std::string& filepath)
	{
		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Key << "Scene" << YAML::Value << "Unnamed Scene";
		out << YAML::Key << "Entities" << YAML::Value << YAML::BeginSeq;
		for (const auto entityId : m_Scene->m_Registry.view<entt::entity>())
		{
			const Entity entity{ entityId, m_Scene.get() };
			if (!entity)
				continue;

			SerializeEntity(out, entity);
		}
		out << YAML::EndSeq;
		out << YAML::EndMap;

		const std::filesystem::path filePathObj(filepath);

		/// Check if there's a parent directory
		if (const std::filesystem::path parentDir = filePathObj.parent_path(); !parentDir.empty()) {
			std::error_code ec;
			std::filesystem::create_directories(parentDir, ec);
			if (ec) {
				KBR_CORE_ERROR("Could not create directory {0}: {1}", parentDir.string(), ec.message());
			}
		}

		if (std::ofstream fileOut(filepath); fileOut)
		{
			fileOut << out.c_str();
			fileOut.close();
		}
		else
		{
			KBR_CORE_ERROR("Could not open file {0} for writing!", filepath);
		}
	}

	void SceneSerializer::SerializeRuntime(const std::string& filepath)
	{
		throw std::logic_error("Not implemented");
	}

	bool SceneSerializer::Deserialize(const std::string& filepath)
	{
		throw std::logic_error("Not implemented");
	}

	bool SceneSerializer::DeserializeRuntime(const std::string& filepath)
	{
		throw std::logic_error("Not implemented");
	}
}
