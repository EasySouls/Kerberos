#pragma once

#include <string>
#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

#include "Kerberos/Renderer/Mesh.h"
#include "Kerberos/Renderer/Texture.h"
#include "Kerberos/Renderer/Light.h"
#include "Kerberos/Renderer/Material.h"
#include "Kerberos/Renderer/TextureCube.h"
#include "Kerberos/Scene/SceneCamera.h"
#include "Kerberos/Core/UUID.h"

namespace Kerberos
{
	class ScriptableEntity;

	struct IDComponent
	{
		UUID ID;

		IDComponent() = default;
		~IDComponent() = default;
		IDComponent(const IDComponent&) = default;
		IDComponent(IDComponent&&) = default;
		IDComponent& operator=(const IDComponent&) = default;
		IDComponent& operator=(IDComponent&&) = default;
	};

	struct TransformComponent
	{
		glm::vec3 Translation = glm::vec3(0.0f);
		glm::vec3 Rotation = glm::vec3(0.0f);
		glm::vec3 Scale = glm::vec3(1.0f);

		glm::mat4 WorldTransform = glm::mat4(1.0f);

		TransformComponent() = default;
		~TransformComponent() = default;
		TransformComponent(const TransformComponent&) = default;
		TransformComponent(TransformComponent&&) = default;
		TransformComponent& operator=(const TransformComponent&) = default;
		TransformComponent& operator=(TransformComponent&&) = default;

		explicit TransformComponent(const glm::vec3& translation)
			: Translation(translation)
		{}

		glm::mat4 GetTransform() const
		{
			const glm::mat4 rotation = glm::toMat4(glm::quat(Rotation));

			const glm::mat4 transform = glm::translate(glm::mat4(1.0f), Translation) 
				* rotation 
				* glm::scale(glm::mat4(1.0f), Scale);

			return transform;
		}
	};

	struct SpriteRendererComponent
	{
		glm::vec4 Color{ 1.0f, 1.0f, 1.0f, 1.0f };

		SpriteRendererComponent() = default;
		~SpriteRendererComponent() = default;
		SpriteRendererComponent(const SpriteRendererComponent&) = default;
		SpriteRendererComponent(SpriteRendererComponent&&) = default;
		SpriteRendererComponent& operator=(const SpriteRendererComponent&) = default;
		SpriteRendererComponent& operator=(SpriteRendererComponent&&) = default;

		explicit SpriteRendererComponent(const glm::vec4& color)
			: Color(color)
		{}
	};

	struct TagComponent
	{
		std::string Tag;

		TagComponent() = default;
		~TagComponent() = default;
		TagComponent(const TagComponent&) = default;
		TagComponent(TagComponent&&) = default;
		TagComponent& operator=(const TagComponent&) = default;
		TagComponent& operator=(TagComponent&&) = default;
		TagComponent& operator=(const std::string& tag)
		{
			Tag = tag;
			return *this;
		}

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

	struct StaticMeshComponent
	{
		Ref<Mesh> StaticMesh = nullptr;
		Ref<Material> MeshMaterial = nullptr;
		Ref<Texture2D> MeshTexture = nullptr;
		//AssetHandle MeshTexture;
		bool Visible = true;

		StaticMeshComponent()
		{
			// TODO: This creates a brand-new material and mesh every time. We should probably have a default material and mesh in the renderer and use that instead.
			MeshMaterial = CreateRef<Material>();
			StaticMesh = Mesh::CreateCube(1.0f);
		}

		StaticMeshComponent(const Ref<Mesh>& mesh, const Ref<Material>& material, const Ref<Texture2D>& texture = nullptr)
			: StaticMesh(mesh), MeshMaterial(material), MeshTexture(texture)
		{}
		StaticMeshComponent(const StaticMeshComponent&) = default;
	};

	struct DirectionalLightComponent
	{
		DirectionalLight Light;
		bool IsEnabled = true;

		DirectionalLightComponent() = default;
		explicit DirectionalLightComponent(const DirectionalLight& light)
			: Light(light)
		{}
	};

	struct PointLightComponent
	{
		PointLight Light;
		bool IsEnabled = true;

		PointLightComponent() = default;
		explicit PointLightComponent(const PointLight& light)
			: Light(light)
		{}
	};

	struct SpotLightComponent
	{
		SpotLight Light;
		bool IsEnabled = true;

		SpotLightComponent() = default;
		explicit SpotLightComponent(const SpotLight& light)
			: Light(light)
		{}
	};

	struct SkyboxComponent
	{
		Ref<TextureCube> SkyboxTexture = nullptr;
		Ref<Shader> SkyboxShader = nullptr;

		SkyboxComponent() = default;
		explicit SkyboxComponent(const Ref<TextureCube>& texture)
			: SkyboxTexture(texture)
		{}
		SkyboxComponent(const Ref<TextureCube>& texture, const Ref<Shader>& shader)
			: SkyboxTexture(texture), SkyboxShader(shader)
		{}
	};

	struct HierarchyComponent
	{
		UUID Parent = UUID::Invalid();
		std::vector<UUID> Children;

		HierarchyComponent() = default;
	};

	struct RigidBody3DComponent {

		enum class BodyType : uint8_t
		{
			Static,
			Dynamic,
			Kinematic
		};
		BodyType Type = BodyType::Dynamic;
		float Mass = 1.0f;
		glm::vec3 Velocity = glm::vec3(0.0f);
		glm::vec3 AngularVelocity = glm::vec3(0.0f);
		bool UseGravity = true;
		/// Between 0 and 1
		float Friction = 0.5f;
		/// Between 0 and 1
		float Restitution = 0.5f;

		/// Pointer to the physics engine's runtime body
		void* RuntimeBody = nullptr; 

		RigidBody3DComponent() = default;
		explicit RigidBody3DComponent(const BodyType type)
			: Type(type)
		{}
	};

	struct BoxCollider3DComponent
	{
		glm::vec3 Size = glm::vec3(1.0f);
		glm::vec3 Offset = glm::vec3(0.0f);
		bool IsTrigger = false;

		// Pointer to the physics engine's runtime collider
		void* RuntimeCollider = nullptr; 

		BoxCollider3DComponent() = default;
		explicit BoxCollider3DComponent(const glm::vec3& size, const glm::vec3& offset = glm::vec3(0.0f), const bool isTrigger = false)
			: Size(size), Offset(offset), IsTrigger(isTrigger)
		{}
	};
}
