#pragma once

#include "Kerberos/Core.h"
#include "Kerberos/Scene/Scene.h"

#include <Jolt/Jolt.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Collision/Shape/Shape.h>
#include <Jolt/Physics/Body/BodyActivationListener.h>



namespace Kerberos
{
	class PhysicsSystem
	{
	public:
		PhysicsSystem() = default;
		~PhysicsSystem();

		void Initialize();
		void Update(float deltaTime);

		void Cleanup();
	private:
		void UpdatePhysicsBodies();
		void CreatePhysicsBody(const Entity& entity);
		JPH::RefConst<JPH::Shape> CreateShapeForEntity(const Entity& entity);

		void SyncTransforms();

	private:
		Ref<Scene> m_Scene = nullptr;

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