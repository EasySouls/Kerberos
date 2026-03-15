#pragma once

#include "Kerberos/Core.h"
#include "Kerberos/Scene/Entity.h"
#include "ScriptClass.h"

namespace Kerberos 
{
	/*
	* A runtime instance of a ScriptClass.
	* In the .NET hosting model, instance management is delegated to the managed ScriptGlue bridge.
	* The C++ side holds metadata and the entity ID used to communicate with the managed side.
	*/
	class ScriptInstance
	{
	public:
		explicit ScriptInstance(const Ref<ScriptClass>& scriptClass, Entity entity, const std::unordered_map<std::string, ScriptFieldInitializer>& initialFieldValues);

		void InvokeOnCreate() const;
		void InvokeOnUpdate(float deltaTime) const;


		template<typename T>
		T GetFieldValue(const std::string& name) const
		{
			static_assert(sizeof(T) <= maxFieldSize, "ScriptInstance can only get field types of size 16 or smaller");

			if (!GetFieldValueInternal(name, s_FieldValueBuffer)) {
				return T();
			}
			return *reinterpret_cast<T*>(s_FieldValueBuffer);
		}

		template<typename T>
		void SetFieldValue(const std::string& name, const T& value) const
		{
			static_assert(sizeof(T) <= maxFieldSize, "ScriptInstance can only set field types of size 16 or smaller");

			SetFieldValueInternal(name, &value);
		}

		const Ref<ScriptClass>& GetScriptClass() const { return m_ScriptClass; }

	private:
		void InitializeValues(const std::unordered_map<std::string, ScriptFieldInitializer>& values) const;

		bool	GetFieldValueInternal(const std::string& name, void* buffer) const;
		void	SetFieldValueInternal(const std::string& name, const void* value) const;

	private:
		Entity m_Entity;
		UUID m_EntityID;
		Ref<ScriptClass> m_ScriptClass = nullptr;

		/// 16 is the size of the largest supported field type (double, long, ulong, vec4)
		/// When lists are supported, this will need to be changed
		inline static char s_FieldValueBuffer[maxFieldSize];
	};
}