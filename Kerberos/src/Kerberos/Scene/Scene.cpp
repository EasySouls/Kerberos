#include "kbrpch.h"
#include "Scene.h"
#include "Kerberos/Scene/Entity.h"
#include "Kerberos/Scene/ScriptableEntity.h"

#include "Components.h"
#include "Kerberos/Renderer/Renderer2D.h"
#include "Kerberos/Renderer/Renderer3D.h"
#include "Kerberos/Physics/Utils.h"

#include <Jolt/Jolt.h>
#include <Jolt/RegisterTypes.h>
#include <Jolt/Core/Factory.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Physics/PhysicsSettings.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Body/BodyActivationListener.h>
#include "Jolt/Physics/Collision/Shape/BoxShape.h"

#include <glm/gtx/matrix_decompose.hpp>

#include <algorithm>

#include "Jolt/Physics/Collision/Shape/CapsuleShape.h"
#include "Jolt/Physics/Collision/Shape/ConvexHullShape.h"
#include "Jolt/Physics/Collision/Shape/MeshShape.h"
#include "Jolt/Physics/Collision/Shape/SphereShape.h"
#include "Kerberos/Assets/AssetManager.h"
#include "Kerberos/Renderer/RenderCommand.h"

import Components.PhysicsComponents;

#define USE_MAP_FOR_UUID 1

namespace Kerberos
{
	static JPH::EMotionType GetBodyTypeFromComponent(const RigidBody3DComponent::BodyType& type)
	{
		switch (type)
		{
		case RigidBody3DComponent::BodyType::Static:
			return JPH::EMotionType::Static;
		case RigidBody3DComponent::BodyType::Kinematic:
			return JPH::EMotionType::Kinematic;
		case RigidBody3DComponent::BodyType::Dynamic:
			return JPH::EMotionType::Dynamic;
		}

		KBR_CORE_ASSERT(false, "Unknown body type!");
		return JPH::EMotionType::Static;
	}

	namespace PhysicsTemp
	{
		// Layer that objects can be in, determines which other objects it can collide with
		// Typically you at least want to have 1 layer for moving bodies and 1 layer for static bodies, but you can have more
		// layers if you want. E.g. you could have a layer for high detail collision (which is not used by the physics simulation
		// but only if you do collision testing).
		namespace Layers
		{
			static constexpr JPH::ObjectLayer NON_MOVING = 0;
			static constexpr JPH::ObjectLayer MOVING = 1;
			static constexpr JPH::ObjectLayer NUM_LAYERS = 2;
		};

		/// Class that determines if two object layers can collide
		class ObjectLayerPairFilterImpl : public JPH::ObjectLayerPairFilter
		{
		public:
			virtual bool ShouldCollide(const JPH::ObjectLayer inObject1, const JPH::ObjectLayer inObject2) const override
			{
				switch (inObject1)
				{
				case Layers::NON_MOVING:
					return inObject2 == Layers::MOVING; // Non moving only collides with moving
				case Layers::MOVING:
					return true; // Moving collides with everything
				default:
					JPH_ASSERT(false);
					return false;
				}
			}
		};

		// Each broadphase layer results in a separate bounding volume tree in the broad phase. You at least want to have
		// a layer for non-moving and moving objects to avoid having to update a tree full of static objects every frame.
		// You can have a 1-on-1 mapping between object layers and broadphase layers (like in this case) but if you have
		// many object layers you'll be creating many broad phase trees, which is not efficient. If you want to fine tune
		// your broadphase layers define JPH_TRACK_BROADPHASE_STATS and look at the stats reported on the TTY.
		namespace BroadPhaseLayers
		{
			static constexpr JPH::BroadPhaseLayer NON_MOVING(0);
			static constexpr JPH::BroadPhaseLayer MOVING(1);
			static constexpr uint32_t NUM_LAYERS(2);
		};

		// BroadPhaseLayerInterface implementation
		// This defines a mapping between object and broadphase layers.
		class BPLayerInterfaceImpl final : public JPH::BroadPhaseLayerInterface
		{
		public:
			BPLayerInterfaceImpl()
			{
				// Create a mapping table from object to broad phase layer
				mObjectToBroadPhase[Layers::NON_MOVING] = BroadPhaseLayers::NON_MOVING;
				mObjectToBroadPhase[Layers::MOVING] = BroadPhaseLayers::MOVING;
			}

			virtual uint32_t GetNumBroadPhaseLayers() const override
			{
				return BroadPhaseLayers::NUM_LAYERS;
			}

			virtual JPH::BroadPhaseLayer			GetBroadPhaseLayer(JPH::ObjectLayer inLayer) const override
			{
				JPH_ASSERT(inLayer < Layers::NUM_LAYERS);
				return mObjectToBroadPhase[inLayer];
			}

#if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)
			virtual const char* GetBroadPhaseLayerName(BroadPhaseLayer inLayer) const override
			{
				switch ((BroadPhaseLayer::Type)inLayer)
				{
				case (BroadPhaseLayer::Type)BroadPhaseLayers::NON_MOVING:	return "NON_MOVING";
				case (BroadPhaseLayer::Type)BroadPhaseLayers::MOVING:		return "MOVING";
				default:													JPH_ASSERT(false); return "INVALID";
				}
			}
#endif // JPH_EXTERNAL_PROFILE || JPH_PROFILE_ENABLED

		private:
			JPH::BroadPhaseLayer mObjectToBroadPhase[Layers::NUM_LAYERS];
		};

		/// Class that determines if an object layer can collide with a broadphase layer
		class ObjectVsBroadPhaseLayerFilterImpl : public JPH::ObjectVsBroadPhaseLayerFilter
		{
		public:
			virtual bool ShouldCollide(const JPH::ObjectLayer inLayer1, const JPH::BroadPhaseLayer inLayer2) const override
			{
				switch (inLayer1)
				{
				case Layers::NON_MOVING:
					return inLayer2 == BroadPhaseLayers::MOVING;
				case Layers::MOVING:
					return true;
				default:
					JPH_ASSERT(false);
					return false;
				}
			}
		};

		// An example contact listener
		class KBRContactListener : public JPH::ContactListener
		{
		public:
			// See: ContactListener
			virtual JPH::ValidateResult	OnContactValidate(const JPH::Body& inBody1, const JPH::Body& inBody2, JPH::RVec3Arg inBaseOffset, const JPH::CollideShapeResult& inCollisionResult) override
			{
				KBR_CORE_TRACE("Contact validate callback: {} - {}", inBody1.GetID().GetIndex(), inBody2.GetID().GetIndex());

				// Allows you to ignore a contact before it is created (using layers to not make objects collide is cheaper!)
				return JPH::ValidateResult::AcceptAllContactsForThisBodyPair;
			}

			virtual void OnContactAdded(const JPH::Body& inBody1, const JPH::Body& inBody2, const JPH::ContactManifold& inManifold, JPH::ContactSettings& ioSettings) override
			{
				KBR_CORE_TRACE("A contact was added: {} - {}", inBody1.GetID().GetIndex(), inBody2.GetID().GetIndex());
			}

			virtual void OnContactPersisted(const JPH::Body& inBody1, const JPH::Body& inBody2, const JPH::ContactManifold& inManifold, JPH::ContactSettings& ioSettings) override
			{
				KBR_CORE_TRACE("A contact was persisted: {} - {}", inBody1.GetID().GetIndex(), inBody2.GetID().GetIndex());
			}

			virtual void OnContactRemoved(const JPH::SubShapeIDPair& inSubShapePair) override
			{
				KBR_CORE_TRACE("A contact was removed: {} - {}", inSubShapePair.GetBody1ID().GetIndex(), inSubShapePair.GetBody2ID().GetIndex());
			}
		};

		// An example activation listener
		class KBRBodyActivationListener : public JPH::BodyActivationListener
		{
		public:
			virtual void OnBodyActivated(const JPH::BodyID& inBodyID, uint64_t inBodyUserData) override
			{
				KBR_CORE_TRACE("A body got activated: {}", inBodyID.GetIndex());
			}

			virtual void OnBodyDeactivated(const JPH::BodyID& inBodyID, uint64_t inBodyUserData) override
			{
				KBR_CORE_TRACE("A body went to sleep: {}", inBodyID.GetIndex());

			}
		};
	}

	static JPH::ObjectLayer GetObjectLayerFromComponent(const RigidBody3DComponent::BodyType& type)
	{
		switch (type)
		{
		case RigidBody3DComponent::BodyType::Static:
			return PhysicsTemp::Layers::NON_MOVING;
		case RigidBody3DComponent::BodyType::Kinematic:
		case RigidBody3DComponent::BodyType::Dynamic:
			return PhysicsTemp::Layers::MOVING;
		}

		KBR_CORE_ASSERT(false, "Unknown body type!");
		return PhysicsTemp::Layers::NON_MOVING;
	}

	Scene::Scene()
	{
		m_Registry = entt::basic_registry();

		m_ShadowMapFramebuffer = Framebuffer::Create(FramebufferSpecification{
			.Width = 1024,
			.Height = 1024,
			.Attachments = {
				{ FramebufferTextureFormat::DEPTH24 }
			}
			});
		m_ShadowMapFramebuffer->SetDebugName("ShadowMapFramebuffer");

		m_EditorFramebuffer = Framebuffer::Create(FramebufferSpecification{
			.Width = 1280,
			.Height = 720,
			.Attachments = {
				{ FramebufferTextureFormat::RGBA8 },
				{ FramebufferTextureFormat::RED_INTEGER },
				{ FramebufferTextureFormat::DEPTH24STENCIL8 }
			}
			});
		m_EditorFramebuffer->SetDebugName("EditorFramebuffer");
	}

	Scene::~Scene()
	{
		/// Destroy all entities in the scene
		m_Registry.clear<entt::entity>();
	}

	static void TraceImpl(const char* inFmt, ...)
	{
		// Format the message
		va_list list;
		va_start(list, inFmt);
		char buffer[1024];
		vsnprintf(buffer, sizeof(buffer), inFmt, list);
		va_end(list);

		KBR_CORE_TRACE("Jolt: {}", buffer);
	}

	static bool AssertFailedImpl(const char* inExpression, const char* inMessage, const char* inFile, uint32_t inLine)
	{
		KBR_CORE_ERROR("{}:{}: ({}) {}", inFile, inLine, inExpression, inMessage != nullptr ? inMessage : "");

		return true;
	};

	static inline glm::vec3 ToGlmVec3(const JPH::RVec3& v)
	{
		return {
			(v.GetX()),
			(v.GetY()),
			(v.GetZ())
		};
	}

	static inline glm::quat ToGlmQuat(const JPH::Quat& q)
	{
		return {
			(q.GetW()),
			(q.GetX()),
			(q.GetY()),
			(q.GetZ())
		};
	}

	static void ApplyJoltTransformToEntity(glm::mat4& worldTransform, const JPH::Body& body)
	{
		KBR_PROFILE_FUNCTION();

		const JPH::RVec3 joltPosition = body.GetPosition();
		const JPH::Quat joltRotation = body.GetRotation();

		glm::vec3 position = ToGlmVec3(joltPosition);
		glm::quat rotation = ToGlmQuat(joltRotation);

		// Decompose the current transform to get the scale
		glm::vec3 scale, skew;
		glm::vec4 perspective;
		glm::quat oldRotation;
		glm::vec3 oldPosition;

		glm::decompose(worldTransform, scale, oldRotation, oldPosition, skew, perspective);

		// Rebuild world transform using physics position & rotation but keep original scale
		glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), position);
		glm::mat4 rotationMatrix = glm::toMat4(rotation);
		glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), scale);

		worldTransform = translationMatrix * rotationMatrix * scaleMatrix;
	}

	void Scene::OnRuntimeStart()
	{
		KBR_PROFILE_FUNCTION();

		SetupPhysics();
	}

	void Scene::OnRuntimeStop()
	{
		CleanupPhysics();
	}

	void Scene::OnUpdateEditor(Timestep ts, const EditorCamera& camera)
	{
		Render3DEditor(camera);
	}

	void Scene::OnUpdateRuntime(Timestep ts)
	{
		KBR_PROFILE_FUNCTION();

		/// Update the scripts
		{
			KBR_PROFILE_SCOPE("Scene::OnUpdateRuntime - Scripts update");

			m_Registry.view<NativeScriptComponent>().each([this, ts](auto entity, const NativeScriptComponent& script)
				{
					if (!script.Instance)
					{
						script.Instantiate();
						script.Instance->m_Entity = Entity{ entity, this };
						script.Instance->OnCreate();
					}

					script.Instance->OnUpdate(ts);
				});
		}

		/// Physics
		{
			KBR_PROFILE_SCOPE("Scene::OnUpdateRuntime - PhysicsTemp update");

			constexpr int collisionSteps = 1;
			m_PhysicsSystem->Update(ts, collisionSteps, m_PhysicsTempAllocator, m_PhysicsJobSystem);

			const auto view = m_Registry.view<RigidBody3DComponent, TransformComponent>();
			for (const auto e : view)
			{
				auto& transform = view.get<TransformComponent>(e);
				const auto& rigidBody = view.get<RigidBody3DComponent>(e);

				if (rigidBody.RuntimeBody)
				{
					const JPH::Body* body = static_cast<JPH::Body*>(rigidBody.RuntimeBody);

					ApplyJoltTransformToEntity(transform.WorldTransform, *body);
				}
			}
		}

		/// Render the scene

		const Camera* mainCamera = nullptr;
		glm::mat4 mainCameraTransform;

		{
			const auto view = m_Registry.view<CameraComponent, TransformComponent>();
			for (const auto entity : view)
			{
				auto [camera, transform] = view.get<CameraComponent, TransformComponent>(entity);
				if (camera.IsPrimary)
				{
					mainCamera = &camera.Camera;
					mainCameraTransform = transform.GetTransform();
					break;
				}
			}
		}

		if (mainCamera)
		{
			if (m_Is3D)
			{
				Render3DRuntime(mainCamera, mainCameraTransform);
			}
			else
			{
				Render2DRuntime(mainCamera, mainCameraTransform);
			}
		}
	}

	Entity Scene::CreateEntity(const std::string& name)
	{
		const auto enttId = m_Registry.create();
		Entity entity = { enttId, this };
		m_RootEntities.insert(enttId);

		entity.AddComponent<TransformComponent>();
		entity.AddComponent<HierarchyComponent>();

		auto& tag = entity.AddComponent<TagComponent>();
		tag.Tag = name.empty() ? "Entity" : name;

		const auto& idComp = entity.AddComponent<IDComponent>();
#if USE_MAP_FOR_UUID
		m_UUIDToEntityMap[idComp.ID] = entity;
#endif

		return entity;
	}

	Entity Scene::CreateEntityWithUUID(const std::string& name, const uint64_t uuid)
	{
		const auto enttId = m_Registry.create();
		Entity entity = { enttId, this };
		m_RootEntities.insert(enttId);

		entity.AddComponent<TransformComponent>();
		entity.AddComponent<HierarchyComponent>();

		auto& idComp = entity.AddComponent<IDComponent>();
		idComp.ID = UUID(uuid);

		auto& tag = entity.AddComponent<TagComponent>();
		tag.Tag = name.empty() ? "Entity" : name;

#if USE_MAP_FOR_UUID
		m_UUIDToEntityMap[idComp.ID] = entity;
#endif

		return entity;
	}

	void Scene::DestroyEntity(const Entity entity)
	{
		const entt::entity enttId = static_cast<entt::entity>(entity);
		if (m_RootEntities.contains(enttId))
		{
			m_RootEntities.erase(enttId);
		}

		/// Destroy all children entities
		const auto children = GetChildren(entity);
		for (const auto& child : children)
		{
			DestroyEntity(child);
		}

		m_Registry.destroy(enttId);
	}

	void Scene::DuplicateEntity(const Entity entity, const bool duplicateChildren)
	{
		KBR_PROFILE_FUNCTION();

		const std::string name = entity.GetComponent<TagComponent>().Tag;
		const std::string newName = name + " Copy";

		Entity newEntity = CreateEntity(newName);

		newEntity.GetComponent<TransformComponent>() = entity.GetComponent<TransformComponent>();

		if (entity.HasComponent<SpriteRendererComponent>())
		{
			newEntity.AddComponent<SpriteRendererComponent>(entity.GetComponent<SpriteRendererComponent>());
		}
		if (entity.HasComponent<CameraComponent>())
		{
			newEntity.AddComponent<CameraComponent>(entity.GetComponent<CameraComponent>());
			auto& camera = newEntity.GetComponent<CameraComponent>();
			camera.Camera.SetViewportSize(m_ViewportWidth, m_ViewportHeight);
		}
		if (entity.HasComponent<NativeScriptComponent>())
		{
			auto& script = entity.GetComponent<NativeScriptComponent>();
			/// Instantiate the script instance, so that script.Instance is not nullptr,
			/// when calling script.Instantiate() in the new entity.
			script.Instantiate();

			auto& newScriptComp = newEntity.AddComponent<NativeScriptComponent>(script);
			newScriptComp.Instantiate = [&]() { newScriptComp.Instance = script.Instance; };
			newScriptComp.Destroy = [&]()
				{
					delete newScriptComp.Instance;
					newScriptComp.Instance = nullptr;
				};
		}
		if (entity.HasComponent<StaticMeshComponent>())
		{
			newEntity.AddComponent<StaticMeshComponent>(entity.GetComponent<StaticMeshComponent>());
		}
		if (entity.HasComponent<RigidBody3DComponent>())
		{
			newEntity.AddComponent<RigidBody3DComponent>(entity.GetComponent<RigidBody3DComponent>());
			auto& rigidBody = newEntity.GetComponent<RigidBody3DComponent>();
			rigidBody.RuntimeBody = nullptr;
		}
		if (entity.HasComponent<BoxCollider3DComponent>())
		{
			newEntity.AddComponent<BoxCollider3DComponent>(entity.GetComponent<BoxCollider3DComponent>());
		}
		if (entity.HasComponent<SphereCollider3DComponent>())
		{
			newEntity.AddComponent<SphereCollider3DComponent>(entity.GetComponent<SphereCollider3DComponent>());
		}
		if (entity.HasComponent<CapsuleCollider3DComponent>())
		{
			newEntity.AddComponent<CapsuleCollider3DComponent>(entity.GetComponent<CapsuleCollider3DComponent>());
		}
		if (entity.HasComponent<MeshCollider3DComponent>())
		{
			newEntity.AddComponent<MeshCollider3DComponent>(entity.GetComponent<MeshCollider3DComponent>());
		}
		if (entity.HasComponent<DirectionalLightComponent>())
		{
			newEntity.AddComponent<DirectionalLightComponent>(entity.GetComponent<DirectionalLightComponent>());
		}
		if (entity.HasComponent<PointLightComponent>())
		{
			newEntity.AddComponent<PointLightComponent>(entity.GetComponent<PointLightComponent>());
		}
		if (entity.HasComponent<SpotLightComponent>())
		{
			newEntity.AddComponent<SpotLightComponent>(entity.GetComponent<SpotLightComponent>());
		}

		if (duplicateChildren)
		{
			const auto children = GetChildren(entity);
			for (const auto& child : children)
			{
				DuplicateEntity(child, true);
				SetParent(child, newEntity, false);
			}
		}
	}

	void Scene::CreateChild(const Entity entity)
	{
		const Entity child = CreateEntity("Unnamed");
		SetParent(child, entity);
	}

	Entity Scene::GetEntityByUUID(const UUID uuid) const
	{
		KBR_PROFILE_FUNCTION();

#if USE_MAP_FOR_UUID

		return m_UUIDToEntityMap.at(uuid);
#else
		const auto view = m_Registry.view<IDComponent>();
		for (auto entity : view)
		{
			auto id = view.get<IDComponent>(entity).ID;
			if (id == uuid)
			{
				return { entity, this };
			}
		}

		return {};
#endif
	}

	void Scene::SetParent(const Entity child, const Entity parent, bool keepWorldTransform)
	{
		RemoveParent(child);

		m_RootEntities.erase(static_cast<entt::entity>(child));

		auto& childHierarchy = child.GetComponent<HierarchyComponent>();
		auto& parentHierarchy = parent.GetComponent<HierarchyComponent>();

		childHierarchy.Parent = parent.GetUUID();
		parentHierarchy.Children.emplace_back(child.GetUUID());
	}

	Entity Scene::GetParent(const Entity child) const
	{
		KBR_PROFILE_FUNCTION();

		const auto& childHierarchy = child.GetComponent<HierarchyComponent>();
		if (childHierarchy.Parent.IsValid())
		{
			return GetEntityByUUID(childHierarchy.Parent);
		}
		return {};
	}

	void Scene::RemoveParent(const Entity child)
	{
		KBR_PROFILE_FUNCTION();

		auto& childHierarchy = child.GetComponent<HierarchyComponent>();
		if (childHierarchy.Parent.IsValid())
		{
			auto& parentHierarchy = GetEntityByUUID(childHierarchy.Parent).GetComponent<HierarchyComponent>();
			const auto it = std::ranges::find(parentHierarchy.Children, child.GetUUID());
			if (it != parentHierarchy.Children.end())
			{
				parentHierarchy.Children.erase(it);
			}

			childHierarchy.Parent = UUID::Invalid();
			/// The child is a root entity now
			m_RootEntities.insert(static_cast<entt::entity>(child));
		}
	}

	std::vector<Entity> Scene::GetChildren(const Entity parent) const
	{
		KBR_PROFILE_FUNCTION();

		const auto& parentHierarchy = parent.GetComponent<HierarchyComponent>();

		std::vector<Entity> children;
		for (const auto& child : parentHierarchy.Children)
		{
			Entity entity = GetEntityByUUID(child);
			children.push_back(entity);
		}
		return children;
	}

	void Scene::OnViewportResize(const uint32_t width, const uint32_t height)
	{
		m_ViewportHeight = height;
		m_ViewportWidth = width;

		/// Resize the non-fixed aspect ratio cameras
		const auto view = m_Registry.view<CameraComponent>();
		for (const auto entity : view)
		{
			auto& cameraComponent = view.get<CameraComponent>(entity);
			if (!cameraComponent.FixedAspectRatio)
			{
				cameraComponent.Camera.SetViewportSize(width, height);
			}
		}
	}

	Ref<Scene> Scene::Copy(const Ref<Scene>& other)
	{
		Ref<Scene> newScene = CreateRef<Scene>();

		newScene->m_ViewportWidth = other->m_ViewportWidth;
		newScene->m_ViewportHeight = other->m_ViewportHeight;

		auto& sourceRegistry = other->m_Registry;
		auto& newRegistry = newScene->m_Registry;

		const auto idView = sourceRegistry.view<IDComponent>();
		for (const auto& entity : idView)
		{
			UUID sourceID = sourceRegistry.get<IDComponent>(entity).ID;
			const auto& tag = sourceRegistry.get<TagComponent>(entity).Tag;
			Entity newEntity = newScene->CreateEntityWithUUID(tag, sourceID);

			/// Copy all components

			newEntity.GetComponent<TransformComponent>() = sourceRegistry.get<TransformComponent>(entity);

			if (sourceRegistry.all_of<SpriteRendererComponent>(entity))
			{
				newEntity.AddComponent<SpriteRendererComponent>(sourceRegistry.get<SpriteRendererComponent>(entity));
			}
			if (sourceRegistry.all_of<CameraComponent>(entity))
			{
				auto& cameraComp = sourceRegistry.get<CameraComponent>(entity);
				newEntity.AddComponent<CameraComponent>(cameraComp);
				newEntity.GetComponent<CameraComponent>().Camera.SetViewportSize(newScene->m_ViewportWidth, newScene->m_ViewportHeight);
			}
			if (sourceRegistry.all_of<NativeScriptComponent>(entity))
			{
				auto& scriptComp = sourceRegistry.get<NativeScriptComponent>(entity);
				/// Instantiate the script instance, so that script.Instance is not nullptr,
				/// when calling script.Instantiate() in the new entity.
				scriptComp.Instantiate();

				auto& newScriptComp = newEntity.AddComponent<NativeScriptComponent>();
				newScriptComp.Instantiate = [&]() { newScriptComp.Instance = scriptComp.Instance; };
				newScriptComp.Destroy = [&]()
					{
						delete newScriptComp.Instance;
						newScriptComp.Instance = nullptr;
					};
			}
			if (sourceRegistry.all_of<StaticMeshComponent>(entity))
			{
				newEntity.AddComponent<StaticMeshComponent>(sourceRegistry.get<StaticMeshComponent>(entity));
			}
			if (sourceRegistry.all_of<RigidBody3DComponent>(entity))
			{
				auto& rigidBodyComp = sourceRegistry.get<RigidBody3DComponent>(entity);
				newEntity.AddComponent<RigidBody3DComponent>(rigidBodyComp);
				newEntity.GetComponent<RigidBody3DComponent>().RuntimeBody = nullptr; // Reset runtime body
			}
			if (sourceRegistry.all_of<BoxCollider3DComponent>(entity))
			{
				newEntity.AddComponent<BoxCollider3DComponent>(sourceRegistry.get<BoxCollider3DComponent>(entity));
			}
			if (sourceRegistry.all_of<SphereCollider3DComponent>(entity))
			{
				newEntity.AddComponent<SphereCollider3DComponent>(sourceRegistry.get<SphereCollider3DComponent>(entity));
			}
			if (sourceRegistry.all_of<CapsuleCollider3DComponent>(entity))
			{
				newEntity.AddComponent<CapsuleCollider3DComponent>(sourceRegistry.get<CapsuleCollider3DComponent>(entity));
			}
			if (sourceRegistry.all_of<MeshCollider3DComponent>(entity))
			{
				newEntity.AddComponent<MeshCollider3DComponent>(sourceRegistry.get<MeshCollider3DComponent>(entity));
			}
			if (sourceRegistry.all_of<DirectionalLightComponent>(entity))
			{
				newEntity.AddComponent<DirectionalLightComponent>(sourceRegistry.get<DirectionalLightComponent>(entity));
			}
			if (sourceRegistry.all_of<PointLightComponent>(entity))
			{
				newEntity.AddComponent<PointLightComponent>(sourceRegistry.get<PointLightComponent>(entity));
			}
			if (sourceRegistry.all_of<SpotLightComponent>(entity))
			{
				newEntity.AddComponent<SpotLightComponent>(sourceRegistry.get<SpotLightComponent>(entity));
			}
			if (sourceRegistry.all_of<EnvironmentComponent>(entity))
			{
				newEntity.AddComponent<EnvironmentComponent>(sourceRegistry.get<EnvironmentComponent>(entity));
			}
		}

		return newScene;
	}


	void Scene::Render2DRuntime(const Camera* mainCamera, const glm::mat4& mainCameraTransform)
	{
		Renderer2D::BeginScene(*mainCamera, mainCameraTransform);

		const auto view = m_Registry.view<TransformComponent, SpriteRendererComponent>();
		for (const auto entity : view)
		{
			auto [transform, sprite] = view.get<TransformComponent, SpriteRendererComponent>(entity);

			Renderer2D::DrawQuad(transform.WorldTransform, sprite.Color);
		}

		Renderer2D::EndScene();
	}

	void Scene::Render3DRuntime(const Camera* mainCamera, const glm::mat4& mainCameraTransform)
	{
		KBR_PROFILE_FUNCTION();

		DirectionalLightComponent* dlc = nullptr;
		const auto sunView = m_Registry.view<DirectionalLightComponent, TransformComponent>();
		for (const auto entity : sunView)
		{
			auto [light, transform] = sunView.get<DirectionalLightComponent, TransformComponent>(entity);
			if (light.IsEnabled)
			{
				dlc = &light;
				break;
			}
		}

		if (ShouldRenderShadows(dlc))
		{
			ShadowMapSettings shadowSettings;
			shadowSettings.Resolution = 1024;
			shadowSettings.OrthoSize = 15.0f;
			shadowSettings.NearPlane = 1.0f;
			shadowSettings.FarPlane = 100.0f;
			shadowSettings.EnableShadows = true;

			Renderer3D::BeginShadowPass(dlc->Light, shadowSettings, m_ShadowMapFramebuffer);

			/// Render all shadow-casting meshes
			const auto meshView = m_Registry.view<StaticMeshComponent, TransformComponent>();
			for (auto entity : meshView)
			{
				auto& meshComp = meshView.get<StaticMeshComponent>(entity);
				auto& transformComp = meshView.get<TransformComponent>(entity);

				if (meshComp.StaticMesh && meshComp.MeshMaterial && meshComp.Visible)
				{
					Renderer3D::SubmitMesh(meshComp.StaticMesh, transformComp.WorldTransform,
						meshComp.MeshMaterial, meshComp.MeshTexture, 1.0f,
						static_cast<int>(entity), meshComp.CastShadows);
				}
			}

			dlc->NeedsUpdate = false;

			Renderer3D::EndPass();
		}

		std::vector<PointLight> pointLights;
		const auto pointLightView = m_Registry.view<PointLightComponent, TransformComponent>();
		for (const auto entity : pointLightView)
		{
			auto [light, transform] = pointLightView.get<PointLightComponent, TransformComponent>(entity);
			if (light.IsEnabled)
			{
				pointLights.push_back(light.Light);
			}
		}

		Ref<TextureCube> skyboxTexture = nullptr;
		const auto skyboxView = m_Registry.view<EnvironmentComponent>();
		for (const auto entity : skyboxView)
		{
			const auto& skybox = skyboxView.get<EnvironmentComponent>(entity);
			if (skybox.IsSkyboxEnabled && skybox.SkyboxTexture)
			{
				skyboxTexture = AssetManager::GetAsset<TextureCube>(skybox.SkyboxTexture);
				break;
			}
		}

		m_EditorFramebuffer->Bind();

		RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1 });
		RenderCommand::Clear();

		/// Clear our entity ID attachment to -1, so when rendering entities they fill that space with their entity ID,
		/// and empty spacces will have -1, signaling that there is no entity.
		/// Used for mouse picking.
		m_EditorFramebuffer->ClearAttachment(1, -1);

		Renderer3D::BeginGeometryPass(*mainCamera, mainCameraTransform, &dlc->Light, pointLights, skyboxTexture);

		const auto view = m_Registry.view<TransformComponent, StaticMeshComponent>();
		for (const auto entity : view)
		{
			auto [transform, mesh] = view.get<TransformComponent, StaticMeshComponent>(entity);

			if (mesh.Visible)
			{
				Renderer3D::SubmitMesh(mesh.StaticMesh, transform.WorldTransform, mesh.MeshMaterial, mesh.MeshTexture);
			}
		}

		Renderer3D::EndScene();
	}

	void Scene::Render3DEditor(const EditorCamera& camera)
	{
		DirectionalLightComponent* dlc = nullptr;
		const auto sunView = m_Registry.view<DirectionalLightComponent, TransformComponent>();
		for (const auto entity : sunView)
		{
			auto [light, transform] = sunView.get<DirectionalLightComponent, TransformComponent>(entity);
			if (light.IsEnabled)
			{
				dlc = &light;
				break;
			}
		}

		if (ShouldRenderShadows(dlc))
		{
			ShadowMapSettings shadowSettings;
			shadowSettings.Resolution = 1024;
			shadowSettings.OrthoSize = 15.0f;
			shadowSettings.NearPlane = 1.0f;
			shadowSettings.FarPlane = 100.0f;
			shadowSettings.EnableShadows = true;

			Renderer3D::BeginShadowPass(dlc->Light, shadowSettings, m_ShadowMapFramebuffer);

			/// Render all shadow-casting meshes
			const auto meshView = m_Registry.view<StaticMeshComponent, TransformComponent>();
			for (auto entity : meshView)
			{
				auto& meshComp = meshView.get<StaticMeshComponent>(entity);
				auto& transformComp = meshView.get<TransformComponent>(entity);

				if (meshComp.StaticMesh && meshComp.MeshMaterial && meshComp.Visible)
				{
					Renderer3D::SubmitMesh(meshComp.StaticMesh, transformComp.WorldTransform,
						meshComp.MeshMaterial, meshComp.MeshTexture, 1.0f,
						static_cast<int>(entity), meshComp.CastShadows);
				}
			}

			dlc->NeedsUpdate = false;

			Renderer3D::EndPass();
		}

		std::vector<PointLight> pointLights;
		const auto pointLightView = m_Registry.view<PointLightComponent, TransformComponent>();
		for (const auto entity : pointLightView)
		{
			auto [light, transform] = pointLightView.get<PointLightComponent, TransformComponent>(entity);
			if (light.IsEnabled)
			{
				pointLights.push_back(light.Light);
			}
		}

		Ref<TextureCube> skyboxTexture = nullptr;
		const auto skyboxView = m_Registry.view<EnvironmentComponent>();
		for (const auto entity : skyboxView)
		{
			const auto& skybox = skyboxView.get<EnvironmentComponent>(entity);
			if (skybox.IsSkyboxEnabled && skybox.SkyboxTexture)
			{
				skyboxTexture = AssetManager::GetAsset<TextureCube>(skybox.SkyboxTexture);
				break;
			}
		}

		m_EditorFramebuffer->Bind();

		RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1 });
		RenderCommand::Clear();

		/// Clear our entity ID attachment to -1, so when rendering entities they fill that space with their entity ID,
		/// and empty spacces will have -1, signaling that there is no entity.
		/// Used for mouse picking.
		m_EditorFramebuffer->ClearAttachment(1, -1);

		Renderer3D::BeginGeometryPass(camera, &dlc->Light, pointLights, skyboxTexture);

		const auto view = m_Registry.view<TransformComponent, StaticMeshComponent>();
		for (const auto entity : view)
		{
			auto [transform, mesh] = view.get<TransformComponent, StaticMeshComponent>(entity);

			if (mesh.Visible)
			{
				Renderer3D::SubmitMesh(mesh.StaticMesh, transform.WorldTransform, mesh.MeshMaterial, mesh.MeshTexture, 1.0f, static_cast<int>(entity), mesh.CastShadows);
			}
		}

		Renderer3D::EndPass();
		Renderer3D::EndScene();
	}

	void Scene::UpdateChildTransforms(const Entity parent, const glm::mat4& parentTransform)
	{
		auto& tsc = parent.GetComponent<TransformComponent>();
		const glm::mat4 localTransform = tsc.GetTransform();
		tsc.WorldTransform = parentTransform * localTransform;

		for (const auto& child : GetChildren(parent))
		{
			UpdateChildTransforms(child, tsc.WorldTransform);
		}
	}

	bool Scene::ShouldRenderShadows(const DirectionalLightComponent* dlc) const
	{
		if (m_OnlyRenderShadowMapIfLightHasChanged)
		{
			return m_EnableShadowMapping && dlc && dlc->IsEnabled && dlc->CastShadows && dlc->NeedsUpdate;
		}
		return m_EnableShadowMapping && dlc && dlc->IsEnabled && dlc->CastShadows;
	}

	Entity Scene::GetPrimaryCameraEntity()
	{
		const auto view = m_Registry.view<CameraComponent>();
		for (const auto entity : view)
		{
			const auto& camera = view.get<CameraComponent>(entity);
			if (camera.IsPrimary)
			{
				return Entity{ entity, this };
			}
		}

		return {};
	}

	void Scene::CalculateEntityTransforms()
	{
		KBR_PROFILE_FUNCTION();

		const auto view = m_Registry.view<TransformComponent>();
		for (const auto id : view)
		{
			const Entity entity{ id, this };
			const UUID parentUUID = entity.GetComponent<HierarchyComponent>().Parent;

			/// Only calculate the transforms of root entities, since the UpdateChildTransforms
			/// function will handle all the children
			if (!parentUUID.IsValid())
			{
				UpdateChildTransforms(entity, glm::mat4(1.0f));
			}
		}
	}

	void Scene::CalculateEntityTransform(const Entity& entity)
	{
		UpdateChildTransforms(entity, glm::mat4(1.0f));
	}

	void Scene::SetupPhysics()
	{
		KBR_PROFILE_FUNCTION();

		InitializePhysicsSystem();

		InitializePhysicsCollidersAndBodies();
	}

	void Scene::InitializePhysicsSystem()
	{
		KBR_PROFILE_FUNCTION();

		// Register allocation hook. In this example we'll just let Jolt use malloc / free but you can override these if you want (see Memory.h).
		// This needs to be done before any other Jolt function is called.
		JPH::RegisterDefaultAllocator();

		// Install trace and assert callbacks
		JPH::Trace = TraceImpl;
		JPH_IF_ENABLE_ASSERTS(JPH::AssertFailed = AssertFailedImpl;);

		// Create a factory, this class is responsible for creating instances of classes based on their name or hash and is mainly used for deserialization of saved data.
		// It is not directly used in this example but still required.
		JPH::Factory::sInstance = new JPH::Factory();

		// Register all physics types with the factory and install their collision handlers with the CollisionDispatch class.
		// If you have your own custom shape types you probably need to register their handlers with the CollisionDispatch before calling this function.
		// If you implement your own default material (PhysicsMaterial::sDefault) make sure to initialize it before this function or else this function will create one for you.
		JPH::RegisterTypes();

		// We need a temp allocator for temporary allocations during the physics update. We're
		// pre-allocating 10 MB to avoid having to do allocations during the physics update.
		// B.t.w. 10 MB is way too much for this example but it is a typical value you can use.
		// If you don't want to pre-allocate you can also use TempAllocatorMalloc to fall back to
		// malloc / free.
		constexpr size_t cTempAllocatorSize = 10 * 1024 * 1024; // 10 MB
		m_PhysicsTempAllocator = new JPH::TempAllocatorImpl(cTempAllocatorSize);

		// We need a job system that will execute physics jobs on multiple threads. Typically
		// you would implement the JobSystem interface yourself and let Jolt PhysicsTemp run on top
		// of your own job scheduler. JobSystemThreadPool is an example implementation.
		m_PhysicsJobSystem = new JPH::JobSystemThreadPool(JPH::cMaxPhysicsJobs, JPH::cMaxPhysicsBarriers, static_cast<int>(std::thread::hardware_concurrency()) - 1);

		// This is the max amount of rigid bodies that you can add to the physics system. If you try to add more you'll get an error.
		// Note: This value is low because this is a simple test. For a real project use something in the order of 65536.
		constexpr uint32_t cMaxBodies = 1024;

		// This determines how many mutexes to allocate to protect rigid bodies from concurrent access. Set it to 0 for the default settings.
		constexpr uint32_t cNumBodyMutexes = 0;

		// This is the max amount of body pairs that can be queued at any time (the broad phase will detect overlapping
		// body pairs based on their bounding boxes and will insert them into a queue for the narrowphase). If you make this buffer
		// too small the queue will fill up and the broad phase jobs will start to do narrow phase work. This is slightly less efficient.
		// Note: This value is low because this is a simple test. For a real project use something in the order of 65536.
		constexpr uint32_t cMaxBodyPairs = 1024;

		// This is the maximum size of the contact constraint buffer. If more contacts (collisions between bodies) are detected than this
		// number then these contacts will be ignored and bodies will start interpenetrating / fall through the world.
		// Note: This value is low because this is a simple test. For a real project use something in the order of 10240.
		constexpr uint32_t cMaxContactConstraints = 1024;

		// Create mapping table from object layer to broadphase layer
		// Note: As this is an interface, PhysicsSystem will take a reference to this so this instance needs to stay alive!
		// Also have a look at BroadPhaseLayerInterfaceTable or BroadPhaseLayerInterfaceMask for a simpler interface.
		m_BroadPhaseLayerInterface = new PhysicsTemp::BPLayerInterfaceImpl();

		// Create class that filters object vs broadphase layers
		// Note: As this is an interface, PhysicsSystem will take a reference to this so this instance needs to stay alive!
		// Also have a look at ObjectVsBroadPhaseLayerFilterTable or ObjectVsBroadPhaseLayerFilterMask for a simpler interface.
		m_ObjectVsBroadPhaseLayerFilter = new PhysicsTemp::ObjectVsBroadPhaseLayerFilterImpl();

		// Create class that filters object vs object layers
		// Note: As this is an interface, PhysicsSystem will take a reference to this so this instance needs to stay alive!
		// Also have a look at ObjectLayerPairFilterTable or ObjectLayerPairFilterMask for a simpler interface.
		m_ObjectVsObjectLayerFilter = new PhysicsTemp::ObjectLayerPairFilterImpl();

		m_PhysicsSystem = new JPH::PhysicsSystem();
		m_PhysicsSystem->Init(cMaxBodies, cNumBodyMutexes, cMaxBodyPairs, cMaxContactConstraints, *m_BroadPhaseLayerInterface, *m_ObjectVsBroadPhaseLayerFilter, *m_ObjectVsObjectLayerFilter);

		// A body activation listener gets notified when bodies activate and go to sleep
		// Note that this is called from a job so whatever you do here needs to be thread safe.
		// Registering one is entirely optional.
		m_BodyActivationListener = new PhysicsTemp::KBRBodyActivationListener();
		m_PhysicsSystem->SetBodyActivationListener(m_BodyActivationListener);

		// A contact listener gets notified when bodies (are about to) collide, and when they separate again.
		// Note that this is called from a job so whatever you do here needs to be thread safe.
		// Registering one is entirely optional.
		m_ContactListener = new PhysicsTemp::KBRContactListener();
		m_PhysicsSystem->SetContactListener(m_ContactListener);
	}

	void Scene::InitializePhysicsCollidersAndBodies()
	{
		KBR_PROFILE_FUNCTION();

		KBR_CORE_ASSERT(m_PhysicsSystem, "Physics system is not initialized!");

		JPH::BodyInterface& bodyInterface = m_PhysicsSystem->GetBodyInterface();
		const auto view = m_Registry.view<RigidBody3DComponent>();
		for (const auto e : view)
		{
			Entity entity = { e, this };
			auto& transform = entity.GetComponent<TransformComponent>();
			auto& rigidBody = view.get<RigidBody3DComponent>(e);

			JPH::ShapeRefC shape = nullptr;
			bool isTrigger = false;
			bool hasToOverrideMassProperties = false;

			if (entity.HasComponent<BoxCollider3DComponent>())
			{
				auto& collider = entity.GetComponent<BoxCollider3DComponent>();
				isTrigger = collider.IsTrigger;

				if (collider.Size.x > 0.0f && collider.Size.y > 0.0f && collider.Size.z > 0.0f)
				{
					JPH::BoxShapeSettings shapeSettings(JPH::Vec3(collider.Size.x, collider.Size.y, collider.Size.z));
					shapeSettings.SetEmbedded();

					JPH::ShapeSettings::ShapeResult shapeResult = shapeSettings.Create();
					if (shapeResult.HasError())
					{
						KBR_CORE_ERROR("Jolt: shape error on entity {}: {}", entity.GetComponent<TagComponent>().Tag, shapeResult.GetError().c_str());
						continue;
					}
					shape = shapeResult.Get();
				}
			}
			if (entity.HasComponent<SphereCollider3DComponent>())
			{
				auto& collider = entity.GetComponent<SphereCollider3DComponent>();
				isTrigger = collider.IsTrigger;
				if (collider.Radius > 0.0f)
				{
					JPH::SphereShapeSettings shapeSettings(collider.Radius);
					shapeSettings.SetEmbedded();
					JPH::ShapeSettings::ShapeResult shapeResult = shapeSettings.Create();
					if (shapeResult.HasError())
					{
						KBR_CORE_ERROR("Jolt: shape error on entity {}: {}", entity.GetComponent<TagComponent>().Tag, shapeResult.GetError().c_str());
						continue;
					}
					shape = shapeResult.Get();
				}
			}
			if (entity.HasComponent<CapsuleCollider3DComponent>())
			{
				auto& collider = entity.GetComponent<CapsuleCollider3DComponent>();
				isTrigger = collider.IsTrigger;
				if (collider.Radius > 0.0f && collider.Height > 0.0f)
				{
					JPH::CapsuleShapeSettings shapeSettings(collider.Height * 0.5f, collider.Radius);
					shapeSettings.SetEmbedded();
					JPH::ShapeSettings::ShapeResult shapeResult = shapeSettings.Create();
					if (shapeResult.HasError())
					{
						KBR_CORE_ERROR("Jolt: shape error on entity {}: {}", entity.GetComponent<TagComponent>().Tag, shapeResult.GetError().c_str());
						continue;
					}
					shape = shapeResult.Get();
				}
			}
			if (entity.HasComponent<MeshCollider3DComponent>())
			{
				auto& collider = entity.GetComponent<MeshCollider3DComponent>();
				isTrigger = collider.IsTrigger;

				if (collider.Mesh)
				{
					Ref<Mesh> mesh = AssetManager::GetAsset<Mesh>(collider.Mesh->GetHandle());
					if (!mesh)
					{
						KBR_CORE_ERROR("MeshCollider3DComponent on entity {} does not have a valid mesh!", entity.GetComponent<TagComponent>().Tag);
						continue;
					}

					shape = Physics::Utils::CreateJoltMeshShape(mesh, entity.GetComponent<TagComponent>().Tag);

					hasToOverrideMassProperties = true;
				}
			}

			if (!shape)
			{
				KBR_CORE_ERROR("RigidBody3DComponent on entity {} does not have a valid shape!", entity.GetComponent<TagComponent>().Tag);
				continue;
			}

			const glm::vec4 worldPos = transform.WorldTransform[3];
			const float posX = worldPos.x; //+ collider.Offset.x;
			const float posY = worldPos.y; //+ collider.Offset.y;
			const float posZ = worldPos.z; //+ collider.Offset.z;

			const JPH::Vec3 position = JPH::Vec3(posX, posY, posZ);

			/// Extract rotation quaternion from the transform matrix
			glm::quat glmRotation = glm::quat_cast(glm::mat3(transform.WorldTransform));
			glmRotation = glm::normalize(glmRotation); /// Ensure the quaternion is normalized
			const JPH::Quat rotation = JPH::Quat(glmRotation.x, glmRotation.y, glmRotation.z, glmRotation.w);

			JPH::BodyCreationSettings bodySettings(shape, position, rotation, GetBodyTypeFromComponent(rigidBody.Type), GetObjectLayerFromComponent(rigidBody.Type));
			bodySettings.mIsSensor = isTrigger;

			if (hasToOverrideMassProperties)
			{
				bodySettings.mOverrideMassProperties = JPH::EOverrideMassProperties::CalculateInertia;
				auto massProperties = JPH::MassProperties();
				massProperties.mMass = rigidBody.Mass;
				bodySettings.mMassPropertiesOverride = massProperties;
			}

			JPH::Body* body = bodyInterface.CreateBody(bodySettings);

			body->SetRestitution(rigidBody.Restitution);
			body->SetFriction(rigidBody.Friction);

			rigidBody.RuntimeBody = body;

			const auto& bodyID = body->GetID();

			bodyInterface.AddBody(bodyID, JPH::EActivation::Activate);
		}
	}

	void Scene::CleanupPhysics()
	{
		const auto view = m_Registry.view<RigidBody3DComponent>();
		for (const auto e : view)
		{
			auto& rigidbody = view.get<RigidBody3DComponent>(e);
			rigidbody.RuntimeBody = nullptr;
		}

		delete m_ContactListener;
		m_ContactListener = nullptr;

		delete m_BodyActivationListener;
		m_BodyActivationListener = nullptr;

		delete m_PhysicsSystem;
		m_PhysicsSystem = nullptr;

		delete m_ObjectVsObjectLayerFilter;
		m_ObjectVsObjectLayerFilter = nullptr;

		delete m_ObjectVsBroadPhaseLayerFilter;
		m_ObjectVsBroadPhaseLayerFilter = nullptr;

		delete m_BroadPhaseLayerInterface;
		m_BroadPhaseLayerInterface = nullptr;

		delete m_PhysicsJobSystem;
		m_PhysicsJobSystem = nullptr;

		delete m_PhysicsTempAllocator;
		m_PhysicsTempAllocator = nullptr;

		delete JPH::Factory::sInstance;
		JPH::Factory::sInstance = nullptr;

		JPH::Trace = nullptr;
		JPH_IF_ENABLE_ASSERTS(JPH::AssertFailed = nullptr;);
	}

	template <typename T>
	void Scene::OnComponentAdded(Entity entity, T& component)
	{
		static_assert(false, "No template specialization found for this type");
	}

	template <>
	void Scene::OnComponentAdded<CameraComponent>(Entity entity, CameraComponent& component)
	{
		component.Camera.SetViewportSize(m_ViewportWidth, m_ViewportHeight);
	}

	template <>
	void Scene::OnComponentAdded<IDComponent>(Entity entity, IDComponent& component)
	{}

	template <>
	void Scene::OnComponentAdded<TransformComponent>(Entity entity, TransformComponent& component)
	{}

	template <>
	void Scene::OnComponentAdded<TagComponent>(Entity entity, TagComponent& component)
	{}

	template <>
	void Scene::OnComponentAdded<SpriteRendererComponent>(Entity entity, SpriteRendererComponent& component)
	{}

	template <>
	void Scene::OnComponentAdded<NativeScriptComponent>(Entity entity, NativeScriptComponent& component)
	{}

	template <>
	void Scene::OnComponentAdded<StaticMeshComponent>(Entity entity, StaticMeshComponent& component)
	{}

	template <>
	void Scene::OnComponentAdded<DirectionalLightComponent>(Entity entity, DirectionalLightComponent& component)
	{}

	template <>
	void Scene::OnComponentAdded<PointLightComponent>(Entity entity, PointLightComponent& component)
	{}

	template <>
	void Scene::OnComponentAdded<SpotLightComponent>(Entity entity, SpotLightComponent& component)
	{}

	template <>
	void Scene::OnComponentAdded<HierarchyComponent>(Entity entity, HierarchyComponent& component)
	{}

	template <>
	void Scene::OnComponentAdded<RigidBody3DComponent>(Entity entity, RigidBody3DComponent& component)
	{}

	template <>
	void Scene::OnComponentAdded<BoxCollider3DComponent>(Entity entity, BoxCollider3DComponent& component)
	{}

	template <>
	void Scene::OnComponentAdded<SphereCollider3DComponent>(Entity entity, SphereCollider3DComponent& component)
	{}

	template <>
	void Scene::OnComponentAdded<CapsuleCollider3DComponent>(Entity entity, CapsuleCollider3DComponent& component)
	{}

	template <>
	void Scene::OnComponentAdded<MeshCollider3DComponent>(const Entity entity, MeshCollider3DComponent& component)
	{
		const auto& smc = entity.GetComponent<StaticMeshComponent>();
		if (smc.StaticMesh == nullptr)
		{
			KBR_CORE_ERROR("MeshCollider3DComponent on entity {} does not have a valid static mesh!", entity.GetComponent<TagComponent>().Tag);
			return;
		}

		component.Mesh = smc.StaticMesh;
	}

	template <>
	void Scene::OnComponentAdded<EnvironmentComponent>(Entity entity, EnvironmentComponent& component)
	{}
}
