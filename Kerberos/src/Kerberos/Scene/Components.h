#pragma once

#include <glm/glm.hpp>

#include "Kerberos/Scene/SceneCamera.h"

namespace Kerberos
{
	class ScriptableEntity;

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
