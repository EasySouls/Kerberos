#pragma once

#include <glm/glm.hpp>

#include "Kerberos/Core.h"

namespace Kerberos
{
	struct TransformComponent
	{
		glm::mat4 Transform = glm::mat4(1.0f);

		TransformComponent() = default;
		TransformComponent(const TransformComponent&) = default;

		explicit TransformComponent(const glm::mat4& transform)
			: Transform(transform)
		{}

		explicit operator glm::mat4& () { return Transform; }
		explicit operator const glm::mat4& () const { return Transform; }
	};

	struct SpriteRendererComponent
	{
		glm::vec4 Color{ 1.0f, 1.0f, 1.0f, 1.0f };

		SpriteRendererComponent() = default;
		SpriteRendererComponent(const SpriteRendererComponent&) = default;

		explicit SpriteRendererComponent(const glm::vec4& color)
			: Color(color)
		{}
	};
}