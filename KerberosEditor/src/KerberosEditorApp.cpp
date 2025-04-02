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
		return new KerberosEditorApp();
	}
}