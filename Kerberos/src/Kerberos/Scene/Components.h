#pragma once

#include <glm/glm.hpp>

#include "Kerberos/Renderer/Camera.h"

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

	struct TagComponent
	{
		std::string Tag;

		TagComponent() = default;
		TagComponent(const TagComponent&) = default;

		explicit TagComponent(std::string tag)
			: Tag(std::move(tag)) {}

		explicit operator std::string& () { return Tag; }
		explicit operator const std::string& () const { return Tag; }
	};

	struct CameraComponent
	{
		Camera Camera;
		bool IsPrimary = true;

		explicit CameraComponent(const glm::mat4& projection)
			: Camera(projection)
		{}

		explicit CameraComponent(const Kerberos::Camera& camera)
			: Camera(camera)
		{}
		CameraComponent(const CameraComponent&) = default;
		CameraComponent(CameraComponent&&) = default;

		CameraComponent& operator=(const CameraComponent&) = default;
		CameraComponent& operator=(CameraComponent&&) = default;
		CameraComponent& operator=(const Kerberos::Camera& camera)
		{
			Camera = camera;
			return *this;
		}

		explicit operator Kerberos::Camera& () { return Camera; }

		~CameraComponent() = default;
	};
}
