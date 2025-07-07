#pragma once

#include "Kerberos/Assets/Asset.h"
#include <glm/glm.hpp>

#include "Texture.h"

namespace Kerberos
{
	class Shader;

	struct Material final : Asset
	{
		std::string Name;
		glm::vec3 Ambient = glm::vec3{ 0.1f };
		glm::vec3 Diffuse = glm::vec3{ 1.0f };
		glm::vec3 Specular = glm::vec3{ 0.1f };
		float Shininess = 10.f;

		Ref<Texture2D> DiffuseTexture = nullptr;

		Ref<Shader> MaterialShader = nullptr;

		Material() = default;

		Material(const glm::vec3 ambient, const glm::vec3 diffuse, const glm::vec3 specular, const float shininess)
			: Ambient(ambient), Diffuse(diffuse), Specular(specular), Shininess(shininess)
		{}

		Material(const glm::vec3 ambient, const glm::vec3 diffuse, const glm::vec3 specular, const float shininess, const Ref<Shader>& shader)
			: Ambient(ambient), Diffuse(diffuse), Specular(specular), Shininess(shininess), MaterialShader(shader)
		{}

		AssetType GetType() override { return AssetType::Material; }
	};
}
