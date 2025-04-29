#pragma once

#include <string>
#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>

#include "Kerberos/Scene/SceneCamera.h"

namespace Kerberos
{
	class ScriptableEntity;

	struct TransformComponent
	{
		glm::vec3 Translation = glm::vec3(0.0f);
		glm::vec3 Rotation = glm::vec3(0.0f);
		glm::vec3 Scale = glm::vec3(1.0f);

		TransformComponent() = default;
		TransformComponent(const TransformComponent&) = default;

		explicit TransformComponent(const glm::vec3& translation)
			: Translation(translation)
		{}

		glm::mat4 GetTransform() const
		{
			glm::mat4 transform = glm::translate(glm::mat4(1.0f), Translation);

			transform = glm::rotate(transform, glm::radians(Rotation.x), glm::vec3(1, 0, 0));
			transform = glm::rotate(transform, glm::radians(Rotation.y), glm::vec3(0, 1, 0));
			transform = glm::rotate(transform, glm::radians(Rotation.z), glm::vec3(0, 0, 1));

			transform = glm::scale(transform, Scale);

			return transform;
		}
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
		explicit operator const char* () const { return Tag.c_str(); }
	};

	struct CameraComponent
	{
		SceneCamera Camera;
		bool IsPrimary = true;

		/**
		* If true, the camera will maintain a fixed aspect ratio when the window is resized.
		*/
		bool FixedAspectRatio = false;

		CameraComponent() = default;
		explicit CameraComponent(const SceneCamera& camera)
			: Camera(camera)
		{}
		CameraComponent(const CameraComponent&) = default;
		CameraComponent(CameraComponent&&) = default;

		CameraComponent& operator=(const CameraComponent&) = default;
		CameraComponent& operator=(CameraComponent&&) = default;
		CameraComponent& operator=(const SceneCamera& camera)
		{
			Camera = camera;
			return *this;
		}

		explicit operator SceneCamera& () { return Camera; }

		~CameraComponent() = default;
	};

	struct NativeScriptComponent
	{
		ScriptableEntity* Instance = nullptr;

		std::function<void()> Instantiate;
		std::function<void()> Destroy;

		template<typename T>
		void Bind()
		{
			Instantiate = [&]() { Instance = new T(); };

			Destroy = [&]() { 
				delete reinterpret_cast<T*>(Instance); 
				Instance = nullptr;
			};
		}
	};
}
