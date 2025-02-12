#pragma once

#ifdef KBR_PLATFORM_WINDOWS

extern Kerberos::Application* Kerberos::CreateApplication();

int main(int argc, char** argv)
{
	auto app = Kerberos::CreateApplication();
	app->Run();
	delete app;
}

#endif