#pragma once
#include <glm/glm.hpp>

namespace Kerberos
{
	class Shader;

	struct Material
	{
		glm::vec3 Ambient = glm::vec3{ 0.0f };
		glm::vec3 Diffuse = glm::vec3{ 1.0f };
		glm::vec3 Specular = glm::vec3{ 0.0f };
		float Shininess = 10.f;

		Ref<Shader> Shader = nullptr;

		Material() = default;

		Material(glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 specular, float shininess)
			: Ambient(ambient), Diffuse(diffuse), Specular(specular), Shininess(shininess)
		{}
	};
}