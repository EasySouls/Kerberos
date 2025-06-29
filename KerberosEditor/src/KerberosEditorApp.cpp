#include <Kerberos.h>
#include <Kerberos/EntryPoint.h>

#include "EditorLayer.h"

namespace Kerberos
{
	class KerberosEditorApp : public Application
	{
	public:
		KerberosEditorApp()
			: Application("Kerberos Editor")
		{
			PushLayer(new EditorLayer());
		}

		~KerberosEditorApp() override = default;
	};

	Application* CreateApplication()
	{
		/// This is only needed so when the editor initializes, the static project shouldn't be null
		Project::New();

		return new KerberosEditorApp();
	}
}