#include "kbrpch.h"
#include "ScriptInterface.h"

#include "Kerberos/Log.h"
#include "Kerberos/Core/Input.h"
#include "Kerberos/Core/KeyCodes.h"
#include "Kerberos/Scene/Scene.h"
#include "Kerberos/Scripting/ScriptEngine.h"

#include <mono/jit/jit.h>
#include <glm/glm.hpp>

#include <memory>



namespace Kerberos
{

#define KBR_ADD_INTERNAL_CALL(name) mono_add_internal_call("Kerberos.Source.InternalCalls::" #name, reinterpret_cast<const void*>(name))

	void ScriptInterface::RegisterComponentTypes()
	{
		
	}

	static void NativeLog(MonoString* message)
	{
		char* messageCStr = mono_string_to_utf8(message);
		std::string messageStr(messageCStr);
		mono_free(messageCStr);
		KBR_CORE_INFO("C# Log: {0}", messageStr);
	}

	static void TransformComponent_GetTranslation(const UUID entityID, glm::vec3* outTranslation)
	{
		const std::weak_ptr<Scene> scene = ScriptEngine::GetSceneContext();
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
			const std::weak_ptr<Scene> scene = ScriptEngine::GetSceneContext();
			glm::vec3& currentTranslation = scene.lock()->GetEntityByUUID(entityID).GetComponent<TransformComponent>().Translation;
			currentTranslation = *translation;
		}
	}

	static void TransformComponent_GetRotation(const UUID entityID, glm::vec3* outRotation)
	{
		const std::weak_ptr<Scene> scene = ScriptEngine::GetSceneContext();
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
			const std::weak_ptr<Scene> scene = ScriptEngine::GetSceneContext();
			glm::vec3& currentRotation = scene.lock()->GetEntityByUUID(entityID).GetComponent<TransformComponent>().Rotation;
			currentRotation = *rotation;
		}
	}

	static void TransformComponent_GetScale(const UUID entityID, glm::vec3* outScale)
	{
		const std::weak_ptr<Scene> scene = ScriptEngine::GetSceneContext();
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
			const std::weak_ptr<Scene> scene = ScriptEngine::GetSceneContext();
			glm::vec3& currentScale = scene.lock()->GetEntityByUUID(entityID).GetComponent<TransformComponent>().Scale;
			currentScale = *scale;
		}
	}

	static bool Input_IsKeyDown(const KeyCode key)
	{
		return Input::IsKeyPressed(key);
	}

	void ScriptInterface::RegisterFunctions() 
	{
		KBR_ADD_INTERNAL_CALL(NativeLog);

		KBR_ADD_INTERNAL_CALL(TransformComponent_GetTranslation);
		KBR_ADD_INTERNAL_CALL(TransformComponent_SetTranslation);
		KBR_ADD_INTERNAL_CALL(TransformComponent_GetRotation);
		KBR_ADD_INTERNAL_CALL(TransformComponent_SetRotation);
		KBR_ADD_INTERNAL_CALL(TransformComponent_GetScale);
		KBR_ADD_INTERNAL_CALL(TransformComponent_SetScale);

		KBR_ADD_INTERNAL_CALL(Input_IsKeyDown);
	}
}
