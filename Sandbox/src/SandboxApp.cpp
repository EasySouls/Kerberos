#include <Kerberos.h>
#include <Kerberos/EntryPoint.h>

#include "Sandbox2D.h"

class Sandbox : public Kerberos::Application
{
public:
	Sandbox(const Kerberos::ApplicationSpecification& spec)
		: Application(spec)
	{
		//PushLayer(new ExampleLayer());
		PushLayer(new Sandbox2D());
	}

	~Sandbox() override = default;
};

Kerberos::Application* Kerberos::CreateApplication(const ApplicationCommandLineArgs args)
{
	ApplicationSpecification spec;
	spec.Name = "Sandbox";
	spec.CommandLineArgs = args;

	return new Sandbox(spec);
}