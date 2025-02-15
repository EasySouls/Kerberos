#include <Kerberos.h>

class ExampleLayer : public Kerberos::Layer
{
public:
	ExampleLayer()
		: Layer("Example")
	{
	}
	void OnUpdate() override
	{
		KBR_INFO("ExampleLayer::Update");
	}
	void OnEvent(Kerberos::Event& event) override
	{
		KBR_TRACE("{0}", event.ToString());
	}
};

class Sandbox : public Kerberos::Application
{
public: 
	Sandbox()
	{
		PushLayer(new ExampleLayer());
		PushOverlay(new Kerberos::ImGuiLayer());
	}

	~Sandbox()
	{

	}
};

Kerberos::Application* Kerberos::CreateApplication()
{
	return new Sandbox();
}