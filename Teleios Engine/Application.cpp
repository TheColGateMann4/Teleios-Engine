#include "Application.h"
#include "Macros/ErrorMacros.h"

Application::Application(unsigned int width, unsigned int height, const char* name)
	:
	m_name(name),
	window(width, height, m_name, DXGI_FORMAT_R16G16B16A16_FLOAT)
{

};

Application::~Application()
{

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
	std::cout << "GetCurrentBackBufferIndex(): " << window.graphics.GetCurrentBackBufferIndex() << '\n';

	window.graphics.FinishFrame();
}