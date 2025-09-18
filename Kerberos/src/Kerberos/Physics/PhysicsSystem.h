#pragma once

#include "Kerberos/Core.h"
#include "Kerberos/Core/UUID.h"

namespace JPH
{
	class PhysicsSystem;
	class BodyInterface;
	class TempAllocator;
	class JobSystem;
	class Shape;
	class Body;

	template <class T>
	class RefConst;

	class ObjectVsBroadPhaseLayerFilter;
	class BroadPhaseLayerInterface;
	class ContactListener;
	class BodyActivationListener;
	class ObjectLayerPairFilter;

	class Vec3;
}

namespace Kerberos
{
	class Entity;
	class Scene;

	class PhysicsSystem
	{
	public:
		PhysicsSystem() = default;
		~PhysicsSystem();

		void Initialize(const Ref<Scene>& scene);
		void Update(float deltaTime);

		/// TODO: Might not be a good idea to expose this
		JPH::BodyInterface& GetBodyInterface() const;

		void Cleanup();
	private:
		void UpdateAndCreatePhysicsBodies();
		void CreatePhysicsBody(const Entity& entity);
		JPH::RefConst<JPH::Shape> CreateShapeForEntity(const Entity& entity);

		void SyncTransforms() const;

		static bool IsColliderTrigger(const Entity& entity);

	private:
		std::weak_ptr<Scene> m_Scene;

		std::unordered_map<UUID, JPH::Vec3> m_ColliderOffsets;

		JPH::PhysicsSystem* m_JoltSystem = nullptr;
		JPH::TempAllocator* m_PhysicsTempAllocator = nullptr;
		JPH::JobSystem* m_PhysicsJobSystem = nullptr;
		JPH::ObjectVsBroadPhaseLayerFilter* m_ObjectVsBroadPhaseLayerFilter = nullptr;
		JPH::BroadPhaseLayerInterface* m_BroadPhaseLayerInterface = nullptr;
		JPH::ObjectLayerPairFilter* m_ObjectVsObjectLayerFilter = nullptr;
		JPH::ContactListener* m_ContactListener = nullptr;
		JPH::BodyActivationListener* m_BodyActivationListener = nullptr;

		/**
		 * Shape cache to avoid creating the same shape multiple times.
		 */
		std::unordered_map<size_t, JPH::RefConst<JPH::Shape>> shapeCache;
	};

}