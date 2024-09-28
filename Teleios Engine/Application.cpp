#include "Application.h"
#include "Macros/ErrorMacros.h"

Application::Application(unsigned int width, unsigned int height, const char* name)
{
	INITIALIZE_CONSOLE
};

Application::~Application()
{
	CLEANUP_CONSOLE
}

int Application::Run()
{
	while(true)
	{
		Sleep(1);
	}

	return 0;
}