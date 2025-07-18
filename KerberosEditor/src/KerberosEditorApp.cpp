#include <Kerberos.h>
#include <Kerberos/EntryPoint.h>

#include "EditorLayer.h"

namespace Kerberos
{
	class KerberosEditorApp : public Application
	{
	public:
		explicit KerberosEditorApp(const ApplicationSpecification& spec)
			: Application(spec)
		{
			PushLayer(new EditorLayer());
		}

		~KerberosEditorApp() override = default;
	};

	Application* CreateApplication(const ApplicationCommandLineArgs args)
	{
		ApplicationSpecification spec;
		spec.Name = "Kerberos Editor";
		spec.CommandLineArgs = args;

		/// This is only needed so when the editor initializes, the static project shouldn't be null
		Project::New();

		return new KerberosEditorApp(spec);
	}
}