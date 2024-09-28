#include "Application.h"
#include "Macros/ErrorMacros.h"

Application::Application(unsigned int width, unsigned int height, const char* name)
	:
	m_name(name),
	window(width, height, m_name)
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
		BOOL messageResult = window.HandleMessages();

		if (messageResult != TRUE)
			return messageResult;

		Update();
	}
}

void Application::Update()
{

}