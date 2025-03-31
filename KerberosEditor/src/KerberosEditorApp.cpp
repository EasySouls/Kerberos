#include <Kerberos.h>
#include <Kerberos/EntryPoint.h>

#include "EditorLayer.h"

namespace Kerberos
{
	class HazelEditorApp : public Application
	{
	public:
		HazelEditorApp()
			: Application("Kerberos Editor")
		{
			PushLayer(new EditorLayer());
		}

		~HazelEditorApp() override = default;
	};

	Application* CreateApplication()
	{
		return new HazelEditorApp();
	}
}