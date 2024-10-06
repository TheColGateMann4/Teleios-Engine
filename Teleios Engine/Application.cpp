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
	// synchronise before deletion
}

int Application::Run()
{
	InitializeScene();

	while(true)
	{
		BOOL messageResult = window.HandleMessages();

		if (messageResult != TRUE)
			return messageResult;

		Update();
	}
}

void Application::InitializeScene()
{
	triangle = std::make_shared<Triangle>(window.graphics);
}

void Application::Update()
{
	triangle->Draw(window.graphics);

	window.graphics.FinishFrame();
}