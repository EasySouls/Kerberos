#include <Kerberos.h>

class Sandbox : public Kerberos::Application
{
public: 
	Sandbox()
	{

	}

	~Sandbox()
	{

	}
};

Kerberos::Application* Kerberos::CreateApplication()
{
	return new Sandbox();
}