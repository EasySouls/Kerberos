#pragma once

#ifdef KBR_PLATFORM_WINDOWS

extern Kerberos::Application* Kerberos::CreateApplication();

int main(int argc, char** argv)
{
	Kerberos::Log::Init();
	KBR_CORE_INFO("Core logger initialized");
	KBR_INFO("Client logger initialized");

	auto app = Kerberos::CreateApplication();
	app->Run();
	delete app;
}

#endif