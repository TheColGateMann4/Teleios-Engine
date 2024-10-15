#include "Application.h"
#include "Macros/ErrorMacros.h"
#include <backend/imgui_impl_dx12.h>
#include <backend/imgui_impl_win32.h>
#include <imgui.h>

Application::Application(unsigned int width, unsigned int height, const char* name)
	:
	m_name(name),
	window(width, height, m_name, DXGI_FORMAT_R16G16B16A16_FLOAT)
{

};

Application::~Application()
{
	//window.graphics.WaitForGPU();
}

int Application::Run()
{
	InitializeScene();

	window.graphics.WaitForGPU();

	while(true)
	{
		window.input.Update();

		BOOL messageResult = window.HandleMessages();

		if (messageResult != TRUE)
			return messageResult;

		Update();
	}
}

void Application::InitializeScene()
{
	triangle = std::make_shared<Triangle>(window.graphics);
	camera = std::make_shared<Camera>(window.graphics);
}

void Application::Update()
{
	window.graphics.BeginFrame();


	triangle->DrawImguiWindow(window.graphics);

	camera->DrawImguiWindow();


	triangle->Update(window.graphics, *camera);

	triangle->Draw(window.graphics);

	
	window.graphics.FinishFrame();

	window.graphics.WaitForGPU();
}