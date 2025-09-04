#pragma once

#include <filesystem>
#include <unordered_map>

extern "C" {
	typedef struct _MonoAssembly	MonoAssembly;
	typedef struct _MonoClass		MonoClass;
	typedef struct _MonoObject		MonoObject;
	typedef struct _MonoMethod		MonoMethod;
	typedef struct _MonoDomain		MonoDomain;
	typedef struct _MonoImage		MonoImage;
}

namespace Kerberos { class Scene; }
namespace Kerberos { class Entity; }

namespace Kerberos
{

	class ScriptClass
	{
	public:
		ScriptClass() = default;
		ScriptClass(MonoImage* image, std::string classNamespace, std::string className);

		MonoObject* Instantiate() const;

		MonoMethod* GetMethod(const std::string& name, int paramCount) const;
		MonoObject* InvokeMethod(MonoMethod* method, MonoObject* instance, void** params = nullptr) const;

	private:
		std::string m_ClassNamespace;
		std::string m_ClassName;

		MonoClass* m_MonoClass = nullptr;
	};

	class ScriptInstance
	{
	public:
		explicit ScriptInstance(const Ref<ScriptClass>& scriptClass);

		void InvokeOnCreate() const;
		void InvokeOnUpdate(float deltaTime) const;

	private:
		Ref<ScriptClass> m_ScriptClass = nullptr;

		MonoObject* m_Instance = nullptr;
		MonoMethod* m_OnCreateMethod = nullptr;
		MonoMethod* m_OnUpdateMethod = nullptr;
	};

	class ScriptEngine
	{
	public:
		static void Init();
		static void Shutdown();

		static void OnRuntimeStart(const Ref<Scene>& scene);
		static void OnRuntimeStop();

		static void OnCreateEntity(Entity entity);

		static bool ClassExists(const std::string& className);
		static const std::unordered_map <std::string, Ref<ScriptClass>>& GetEntityClasses();

	private:
		static void InitMono();
		static void ShutdownMono();

		static MonoObject* InstantiateClass(MonoClass* klass);

		static void LoadAssembly(const std::filesystem::path& assemblyPath);
		static MonoAssembly* LoadMonoAssembly(const std::filesystem::path& assemblyPath);
		static void LoadAssemblyClasses(const MonoAssembly* assembly, MonoImage* image);

		friend class ScriptClass;
	};
}
