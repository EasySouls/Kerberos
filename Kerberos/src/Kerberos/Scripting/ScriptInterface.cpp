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

#define KBR_ADD_INTERNAL_CALL(name) mono_add_internal_call("Kerberos.InternalCalls::" #name, reinterpret_cast<const void*>(name))

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

	static void Entity_GetTranslation(const UUID entityID, glm::vec3* outTranslation)
	{
		const std::weak_ptr<Scene> scene = ScriptEngine::GetSceneContext();
		if (outTranslation)
		{
			const glm::vec3 translation = scene.lock()->GetEntityByUUID(entityID).GetComponent<TransformComponent>().Translation;
			*outTranslation = translation;
		}
	}

	static void Entity_SetTranslation(const UUID entityID, const glm::vec3* translation)
	{
		if (translation)
		{
			const std::weak_ptr<Scene> scene = ScriptEngine::GetSceneContext();
			glm::vec3& currentTranslation = scene.lock()->GetEntityByUUID(entityID).GetComponent<TransformComponent>().Translation;
			KBR_CORE_INFO("Setting entity translation to: ({0}, {1}, {2})", translation->x, translation->y, translation->z);
			currentTranslation = *translation;
		}
	}

	static bool Input_IsKeyDown(const KeyCode key)
	{
		return Input::IsKeyPressed(key);
	}

	void ScriptInterface::RegisterFunctions() 
	{
		KBR_ADD_INTERNAL_CALL(NativeLog);

		KBR_ADD_INTERNAL_CALL(Entity_GetTranslation);
		KBR_ADD_INTERNAL_CALL(Entity_SetTranslation);

		KBR_ADD_INTERNAL_CALL(Input_IsKeyDown);
	}
}
