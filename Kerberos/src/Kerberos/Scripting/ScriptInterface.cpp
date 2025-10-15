#include "kbrpch.h"

#include "ScriptInterface.h"

#include "Kerberos/Log.h"
#include "Kerberos/Core/Input.h"
#include "Kerberos/Core/KeyCodes.h"
#include "Kerberos/Scene/Scene.h"
#include "Kerberos/Scripting/ScriptEngine.h"
#include "Kerberos/Scripting/ScriptInstance.h"
#include "Kerberos/Scene/Components.h"
#include "Kerberos/Scene/Components/PhysicsComponents.h"

#include <mono/metadata/reflection.h>
#include <glm/glm.hpp>

#include <Jolt/Jolt.h>
#include <Jolt/Physics/Body/Body.h>

#include <memory>

#include "ScriptUtils.h"

namespace Kerberos
{
#define KBR_ADD_INTERNAL_CALL(name) mono_add_internal_call("Kerberos.Source.InternalCalls::" #name, reinterpret_cast<const void*>(name))

	static std::unordered_map<MonoType*, std::function<bool(Entity)>> s_EntityHasComponentFunctions;

	static void NativeLog(MonoString* message)
	{
		char* messageCStr = mono_string_to_utf8(message);
		std::string messageStr(messageCStr);
		mono_free(messageCStr);
		KBR_CORE_INFO("C# Log: {0}", messageStr);
	}

	static bool Entity_HasComponent(const UUID entityID, MonoReflectionType* componentType)
	{

		if (const std::shared_ptr<Scene> scene = ScriptEngine::GetSceneContext().lock())
		{
			const Entity entity = scene->GetEntityByUUID(entityID);

			MonoType* monoType = mono_reflection_type_get_type(componentType);
			KBR_CORE_ASSERT(monoType && s_EntityHasComponentFunctions.contains(monoType), "Component doesn't exist or hasn't been registered!");

			const auto& hasComponentFunc = s_EntityHasComponentFunctions.at(monoType);
			return hasComponentFunc(entity);
		}
		return false;
	}

	static uint64_t Entity_FindEntityByName(MonoString* name)
	{
		char* nameCStr = mono_string_to_utf8(name);
		const std::string nameStr(nameCStr);
		mono_free(nameCStr);

		if (const std::shared_ptr<Scene> scene = ScriptEngine::GetSceneContext().lock())
		{
			if (const Entity entity = scene->FindEntityByName(nameStr))
			{
				return entity.GetUUID();
			}
		}
		return UUID::Invalid();
	}

	static MonoObject* Entity_GetScriptInstance(const UUID entityID)
	{
		const Ref<ScriptInstance> instance = ScriptEngine::GetEntityInstance(entityID);
		KBR_CORE_ASSERT(instance, "No script instance found for entity!");

		return const_cast<MonoObject*>(instance->GetManagedObject());
	}

	static void TransformComponent_GetTranslation(const UUID entityID, glm::vec3* outTranslation)
	{
		const std::weak_ptr<Scene>& scene = ScriptEngine::GetSceneContext();
		if (outTranslation)
		{
			const glm::vec3 translation = scene.lock()->GetEntityByUUID(entityID).GetComponent<TransformComponent>().Translation;
			*outTranslation = translation;
		}
	}

	static void TransformComponent_SetTranslation(const UUID entityID, const glm::vec3* translation)
	{
		if (translation)
		{
			const std::weak_ptr<Scene>& scene = ScriptEngine::GetSceneContext();
			glm::vec3& currentTranslation = scene.lock()->GetEntityByUUID(entityID).GetComponent<TransformComponent>().Translation;
			currentTranslation = *translation;
		}
	}

	static void TransformComponent_GetRotation(const UUID entityID, glm::vec3* outRotation)
	{
		const std::weak_ptr<Scene>& scene = ScriptEngine::GetSceneContext();
		if (outRotation)
		{
			const glm::vec3 rotation = scene.lock()->GetEntityByUUID(entityID).GetComponent<TransformComponent>().Rotation;
			*outRotation = rotation;
		}
	}

	static void TransformComponent_SetRotation(const UUID entityID, const glm::vec3* rotation)
	{
		if (rotation)
		{
			const std::weak_ptr<Scene>& scene = ScriptEngine::GetSceneContext();
			glm::vec3& currentRotation = scene.lock()->GetEntityByUUID(entityID).GetComponent<TransformComponent>().Rotation;
			currentRotation = *rotation;
		}
	}

	static void TransformComponent_GetScale(const UUID entityID, glm::vec3* outScale)
	{
		const std::weak_ptr<Scene>& scene = ScriptEngine::GetSceneContext();
		if (outScale)
		{
			const glm::vec3 scale = scene.lock()->GetEntityByUUID(entityID).GetComponent<TransformComponent>().Scale;
			*outScale = scale;
		}
	}

	static void TransformComponent_SetScale(const UUID entityID, const glm::vec3* scale)
	{
		if (scale)
		{
			const std::weak_ptr<Scene>& scene = ScriptEngine::GetSceneContext();
			glm::vec3& currentScale = scene.lock()->GetEntityByUUID(entityID).GetComponent<TransformComponent>().Scale;
			currentScale = *scale;
		}
	}

	static void Rigidbody3DComponent_ApplyImpulse(const UUID entityID, const glm::vec3* force)
	{
		if (force)
		{
			const std::weak_ptr<Scene>& scene = ScriptEngine::GetSceneContext();
			const Ref<Scene> currentScene = scene.lock();
			const Entity entity = currentScene->GetEntityByUUID(entityID);

			KBR_CORE_ASSERT(entity.HasComponent<RigidBody3DComponent>(), "Entity doesn't have a Rigidbody3DComponent.");


			const RigidBody3DComponent& rb3d = entity.GetComponent<RigidBody3DComponent>();
			KBR_CORE_ASSERT(rb3d.RuntimeBody, "Rigidbody3DComponent doesn't have a runtime body.");

			const JPH::Body* body = static_cast<JPH::Body*>(rb3d.RuntimeBody);
			const JPH::BodyID& bodyId = body->GetID();
			const uint32_t bodyIdValue = bodyId.GetIndexAndSequenceNumber();

			const IPhysicsSystem& physicsSystem = currentScene->GetPhysicsSystem();
			physicsSystem.AddImpulse(bodyIdValue, *force);
		}
	}

	static void Rigidbody3DComponent_ApplyImpulseAtPoint(const UUID entityID, const glm::vec3* force, const glm::vec3* inPoint)
	{
		if (force)
		{
			const std::weak_ptr<Scene>& scene = ScriptEngine::GetSceneContext();
			const Ref<Scene> currentScene = scene.lock();
			const Entity entity = currentScene->GetEntityByUUID(entityID);

			KBR_CORE_ASSERT(entity.HasComponent<RigidBody3DComponent>(), "Entity doesn't have a Rigidbody3DComponent.");

			const RigidBody3DComponent& rb3d = entity.GetComponent<RigidBody3DComponent>();
			KBR_CORE_ASSERT(rb3d.RuntimeBody, "Rigidbody3DComponent doesn't have a runtime body.");

			const JPH::Body* body = static_cast<JPH::Body*>(rb3d.RuntimeBody);
			const JPH::BodyID& bodyId = body->GetID();
			const uint32_t bodyIdValue = bodyId.GetIndexAndSequenceNumber();

			const IPhysicsSystem& physicsSystem = currentScene->GetPhysicsSystem();
			physicsSystem.AddImpulse(bodyIdValue, *force, *inPoint);
		}
	}

	static MonoString* TextComponent_GetText(const UUID entityID)
	{
		const std::weak_ptr<Scene>& scene = ScriptEngine::GetSceneContext();
		const Entity entity = scene.lock()->GetEntityByUUID(entityID);
		const TextComponent& textComponent = entity.GetComponent<TextComponent>();
		return ScriptEngine::StringToMonoString(textComponent.Text);
	}

	static void TextComponent_SetText(const UUID entityID, MonoString* text)
	{
		KBR_CORE_ASSERT(text == nullptr, "Null pointer passed to text");

		const std::weak_ptr<Scene>& scene = ScriptEngine::GetSceneContext();
		const Entity entity = scene.lock()->GetEntityByUUID(entityID);

		TextComponent& textComponent = entity.GetComponent<TextComponent>();
		textComponent.Text = ScriptUtils::MonoStringToString(text);
	}

	static void TextComponent_GetColor(const UUID entityID, glm::vec4* outColor)
	{
		KBR_CORE_ASSERT(outColor, "Null pointer passed to outColor");

		const std::weak_ptr<Scene>& scene = ScriptEngine::GetSceneContext();
		const glm::vec4 color = scene.lock()->GetEntityByUUID(entityID).GetComponent<TextComponent>().Color;
		*outColor = color;
	}

	static void TextComponent_SetColor(const UUID entityID, const glm::vec4* color)
	{
		KBR_CORE_ASSERT(color, "Null pointer passed to color");

		const std::weak_ptr<Scene>& scene = ScriptEngine::GetSceneContext();
		glm::vec4& currentColor = scene.lock()->GetEntityByUUID(entityID).GetComponent<TextComponent>().Color;
		currentColor = *color;
	}

	static float TextComponent_GetFontSize(const UUID entityID)
	{
		const std::weak_ptr<Scene>& scene = ScriptEngine::GetSceneContext();
		const float fontSize = scene.lock()->GetEntityByUUID(entityID).GetComponent<TextComponent>().FontSize;
		return fontSize;
	}

	static void TextComponent_SetFontSize(const UUID entityID, const float fontSize)
	{
		const std::weak_ptr<Scene>& scene = ScriptEngine::GetSceneContext();
		float& currentFontSize = scene.lock()->GetEntityByUUID(entityID).GetComponent<TextComponent>().FontSize;
		currentFontSize = fontSize;
	}

	static MonoString* TextComponent_GetFontPath(const UUID entityID)
	{
		const std::weak_ptr<Scene>& scene = ScriptEngine::GetSceneContext();
		const Entity entity = scene.lock()->GetEntityByUUID(entityID);

		const TextComponent& textComponent = entity.GetComponent<TextComponent>();
		const std::string fontPath = textComponent.Font->GetFilepath().string();
		return ScriptEngine::StringToMonoString(fontPath);
	}

	static void TextComponent_SetFontPath(const UUID entityID, const MonoString* fontPath)
	{
		KBR_CORE_ASSERT(fontPath == nullptr, "Null pointer passed to fontPath");
		
		throw std::runtime_error("TextComponent_SetFontPath is not implemented yet");
	}

	static bool Input_IsKeyDown(const KeyCode key)
	{
		return Input::IsKeyPressed(key);
	}

	void ScriptInterface::RegisterFunctions() 
	{
		KBR_ADD_INTERNAL_CALL(NativeLog);

		KBR_ADD_INTERNAL_CALL(Entity_HasComponent);
		KBR_ADD_INTERNAL_CALL(Entity_FindEntityByName);
		KBR_ADD_INTERNAL_CALL(Entity_GetScriptInstance);

		KBR_ADD_INTERNAL_CALL(TransformComponent_GetTranslation);
		KBR_ADD_INTERNAL_CALL(TransformComponent_SetTranslation);
		KBR_ADD_INTERNAL_CALL(TransformComponent_GetRotation);
		KBR_ADD_INTERNAL_CALL(TransformComponent_SetRotation);
		KBR_ADD_INTERNAL_CALL(TransformComponent_GetScale);
		KBR_ADD_INTERNAL_CALL(TransformComponent_SetScale);

		KBR_ADD_INTERNAL_CALL(Rigidbody3DComponent_ApplyImpulse);
		KBR_ADD_INTERNAL_CALL(Rigidbody3DComponent_ApplyImpulseAtPoint);

		KBR_ADD_INTERNAL_CALL(TextComponent_GetText);
		KBR_ADD_INTERNAL_CALL(TextComponent_SetText);
		KBR_ADD_INTERNAL_CALL(TextComponent_GetColor);
		KBR_ADD_INTERNAL_CALL(TextComponent_SetColor);
		KBR_ADD_INTERNAL_CALL(TextComponent_GetFontSize);
		KBR_ADD_INTERNAL_CALL(TextComponent_SetFontSize);
		KBR_ADD_INTERNAL_CALL(TextComponent_GetFontPath);
		KBR_ADD_INTERNAL_CALL(TextComponent_SetFontPath);

		KBR_ADD_INTERNAL_CALL(Input_IsKeyDown);
	}

	template<typename Component>
	static void RegisterComponent(MonoImage* coreImage)
	{
		std::string componentName = typeid(Component).name();
		/// TODO: Will only work with MSVC
		componentName = componentName.substr(componentName.find_last_of("::") + 1);
		const std::string componentNamespace = "Kerberos.Source.Kerberos.Scene";
		KBR_CORE_TRACE("Registering component: {0}", componentName);

		const std::string fullName = componentNamespace + "." + componentName;
		MonoType* managedType = mono_reflection_type_from_name(const_cast<char*>(fullName.c_str()), coreImage);
		KBR_CORE_ASSERT(managedType, "Failed to get managed type for {0}", componentName);

		s_EntityHasComponentFunctions[managedType] = [](const Entity entity) { return entity.HasComponent<Component>(); };
	}

	void ScriptInterface::RegisterComponentTypes()
	{
		MonoImage* coreImage = ScriptEngine::GetCoreAssemblyImage();
		KBR_CORE_ASSERT(coreImage, "Core assembly image is null!");

		RegisterComponent<TransformComponent>(coreImage);
		//RegisterComponent<IDComponent>(coreImage);
		//RegisterComponent<SpriteRendererComponent>(coreImage);
		RegisterComponent<TagComponent>(coreImage);
		//RegisterComponent<CameraComponent>(coreImage);
		//RegisterComponent<NativeScriptComponent>(coreImage);
		//RegisterComponent<StaticMeshComponent>(coreImage);
		//RegisterComponent<DirectionalLightComponent>(coreImage);
		//RegisterComponent<PointLightComponent>(coreImage);
		//RegisterComponent<SpotLightComponent>(coreImage);
		//RegisterComponent<HierarchyComponent>(coreImage);
		//RegisterComponent<EnvironmentComponent>(coreImage);
		RegisterComponent<RigidBody3DComponent>(coreImage);
		//RegisterComponent<BoxCollider3DComponent>(coreImage);
		//RegisterComponent<SphereCollider3DComponent>(coreImage);
		//RegisterComponent<CapsuleCollider3DComponent>(coreImage);
		//RegisterComponent<MeshCollider3DComponent>(coreImage);

	}
}
