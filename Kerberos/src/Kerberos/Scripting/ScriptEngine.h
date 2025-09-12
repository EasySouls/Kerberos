#pragma once

#include "Kerberos/Core/UUID.h"
#include "Kerberos/Scene/Entity.h"
#include <filesystem>
#include <unordered_map>
#include <memory>


extern "C" {
	typedef struct _MonoAssembly	MonoAssembly;
	typedef struct _MonoClass		MonoClass;
	typedef struct _MonoObject		MonoObject;
	typedef struct _MonoMethod		MonoMethod;
	typedef struct _MonoDomain		MonoDomain;
	typedef struct _MonoImage		MonoImage;
}

namespace Kerberos { class ScriptClass;		}
namespace Kerberos { class ScriptInterface; }

namespace Kerberos
{
	class ScriptEngine
	{
	public:
		static void Init();
		static void Shutdown();

		static void OnRuntimeStart(const Ref<Scene>& scene);
		static void OnRuntimeStop();

		static void OnCreateEntity(Entity entity);
		static void OnUpdateEntity(Entity entity, float deltaTime);

		static bool ClassExists(const std::string& className);

		static const std::unordered_map <std::string, Ref<ScriptClass>>& GetEntityClasses();
		static std::weak_ptr<Scene> GetSceneContext();

	private:
		static void InitMono();
		static void ShutdownMono();

		static MonoObject* InstantiateClass(MonoClass* klass);

		static void LoadAssembly(const std::filesystem::path& assemblyPath);
		static MonoAssembly* LoadMonoAssembly(const std::filesystem::path& assemblyPath);
		static void LoadAssemblyClasses(const MonoAssembly* assembly, MonoImage* image);

		static MonoImage* GetCoreAssemblyImage();

		friend class ScriptClass;
		friend class ScriptInterface;
	};
}
