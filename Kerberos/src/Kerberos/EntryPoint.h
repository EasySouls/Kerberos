#pragma once

#ifdef KBR_PLATFORM_WINDOWS

extern Kerberos::Application* Kerberos::CreateApplication();

int main(int argc, char** argv)
{
	Kerberos::Log::Init();
	KBR_CORE_INFO("Core logger initialized");
	KBR_INFO("Client logger initialized");

	KBR_PROFILE_BEGIN_SESSION("Startup", "KerberosProfile-Startup.json");
	const auto app = Kerberos::CreateApplication();
	KBR_PROFILE_END_SESSION();

	KBR_PROFILE_BEGIN_SESSION("Runtime", "KerberosProfile-Runtime.json");
	app->Run();
	KBR_PROFILE_END_SESSION();

	KBR_PROFILE_BEGIN_SESSION("Shutdown", "KerberosProfile-Shutdown.json");
	delete app;
	KBR_PROFILE_END_SESSION();

}

#endif