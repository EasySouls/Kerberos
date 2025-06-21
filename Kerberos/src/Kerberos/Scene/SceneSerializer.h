#pragma once

#include "Scene.h"

namespace Kerberos
{
	class SceneSerializer
	{
	public:
		explicit SceneSerializer(const Ref<Scene>& scene)
			: m_Scene(scene) {}

		void Serialize(const std::string& filepath) const;
		void SerializeRuntime(const std::string& filepath);

		bool Deserialize(const std::string& filepath) const;
		bool DeserializeRuntime(const std::string& filepath);

	private:
		Ref<Scene>	m_Scene;
	};
}

