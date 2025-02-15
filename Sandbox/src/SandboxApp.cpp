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
		if (Kerberos::Input::IsKeyPressed(KBR_KEY_TAB))
			KBR_TRACE("Tab key is pressed!");
	}
	void OnEvent(Kerberos::Event& event) override
	{
		if (event.GetEventType() == Kerberos::EventType::KeyPressed)
		{
			const Kerberos::KeyPressedEvent& e = dynamic_cast<Kerberos::KeyPressedEvent&>(event);
			KBR_TRACE("{0}", static_cast<char>(e.GetKeyCode()));
		}
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