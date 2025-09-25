#pragma once

#include "Kerberos/Core.h"

#include <glm/glm.hpp>

namespace Kerberos 
{
	class Scene;

	class IPhysicsSystem 
	{
	public:
		IPhysicsSystem() = default;
		virtual ~IPhysicsSystem() = default;

		IPhysicsSystem(const IPhysicsSystem& other) = default;
		IPhysicsSystem(IPhysicsSystem&& other) noexcept = delete;

		IPhysicsSystem& operator=(const IPhysicsSystem& other) = delete;
		IPhysicsSystem& operator=(IPhysicsSystem&& other) noexcept = delete;

		virtual void Initialize(const Ref<Scene>& scene) = 0;
		virtual void Cleanup() = 0;

		virtual void Update(float deltaTime) = 0;

		virtual void AddImpulse(uint32_t bodyId, const glm::vec3& impulse) const = 0;
		virtual void AddImpulse(uint32_t bodyId, const glm::vec3& impulse, const glm::vec3& point) const = 0;
	};
}
