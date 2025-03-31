#include <Kerberos.h>
#include <Kerberos/EntryPoint.h>

#include "Sandbox2D.h"

class Sandbox : public Kerberos::Application
{
public:
	Sandbox()
		: Application("Sandbox")
	{
		//PushLayer(new ExampleLayer());
		PushLayer(new Sandbox2D());
	}

	~Sandbox() override = default;
};

Kerberos::Application* Kerberos::CreateApplication()
{
	return new Sandbox();
}