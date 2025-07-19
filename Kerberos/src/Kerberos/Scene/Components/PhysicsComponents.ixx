#include "kbrpch.h"

#include "Kerberos/Renderer/Mesh.h"

export module Components.PhysicsComponents;

export namespace Kerberos
{
	struct SphereCollider3DComponent
	{
		float Radius = 1.0f;
		glm::vec3 Offset = glm::vec3(0.0f);
		bool IsTrigger = false;

		// Pointer to the physics engine's runtime collider
		void* RuntimeCollider = nullptr;

		SphereCollider3DComponent() = default;
		explicit SphereCollider3DComponent(const float radius, const glm::vec3& offset = glm::vec3(0.0f), const bool isTrigger = false)
			: Radius(radius), Offset(offset), IsTrigger(isTrigger)
		{}
	};

	struct CapsuleCollider3DComponent
	{
		float Radius = 0.5f;
		float Height = 1.0f;
		glm::vec3 Offset = glm::vec3(0.0f);
		bool IsTrigger = false;

		// Pointer to the physics engine's runtime collider
		void* RuntimeCollider = nullptr;

		CapsuleCollider3DComponent() = default;
		explicit CapsuleCollider3DComponent(const float radius, const float height, const glm::vec3& offset = glm::vec3(0.0f), const bool isTrigger = false)
			: Radius(radius), Height(height), Offset(offset), IsTrigger(isTrigger)
		{}
	};

	struct MeshCollider3DComponent
	{
		Ref<Mesh> Mesh = nullptr;
		glm::vec3 Offset = glm::vec3(0.0f);
		bool IsTrigger = false;
		// Pointer to the physics engine's runtime collider
		void* RuntimeCollider = nullptr;

		MeshCollider3DComponent() = default;
		explicit MeshCollider3DComponent(const Ref<Kerberos::Mesh>& mesh, const glm::vec3& offset = glm::vec3(0.0f), const bool isTrigger = false)
			: Mesh(mesh), Offset(offset), IsTrigger(isTrigger)
		{}
	};
}