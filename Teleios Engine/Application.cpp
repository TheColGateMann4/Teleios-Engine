#include "Application.h"
#include "Macros/ErrorMacros.h"
#include <backend/imgui_impl_dx12.h>
#include <backend/imgui_impl_win32.h>
#include <imgui.h>

Application::Application(unsigned int width, unsigned int height, const char* name)
	:
	m_name(name),
	window(width, height, m_name, DXGI_FORMAT_R16G16B16A16_FLOAT),
	pipeline(window.graphics),
	imguiLayer(window.graphics)
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
	camera = std::make_shared<Camera>(window.graphics);
	pointLight = std::make_shared<PointLight>(window.graphics, pipeline);
	triangle = std::make_shared<Triangle>(window.graphics, pipeline);
}

void Application::Update()
{
	window.graphics.BeginFrame();

	if (imguiLayer.IsVisible())
		if (window.input.GetKeyDown(VK_OEM_3)) // VK_OEM_3 in US standard keyboard is `~ key
			imguiLayer.ToggleDemoVisibility(!imguiLayer.IsDemoVisible());

	if (window.input.GetKeyDown(VK_INSERT))
		imguiLayer.ToggleVisibility(!imguiLayer.IsVisible());

	if(window.input.GetKeyDown(VK_ESCAPE))
	{
		window.LockCursor(!window.GetCursorLocked());
		window.ShowCursor(!window.GetCursorVisibility());
		imguiLayer.ToggleCaptureInput(!window.GetCursorLocked()); // cursorLocked got negated inside lockCursorFunction
	}

	// initializing imgui windows
	{
		window.input.DrawImguiWindow(imguiLayer.IsVisible());

		triangle->DrawImguiWindow(window.graphics, imguiLayer.IsVisible());

		camera->DrawImguiWindow(imguiLayer.IsVisible());

		pointLight->DrawImguiWindow(window.graphics, imguiLayer.IsVisible());

		imguiLayer.DrawDemoWindow();
	}

	// making imgui get its accumulated commands and heaps ready
	imguiLayer.Render();

	// updating objects
	{
		camera->Update(window.input, window.GetCursorLocked());

		triangle->Update(window.graphics, *camera);
	}

	// rendering 
	{
		// opening graphics command list and clearning allocator
		pipeline.GetGraphicCommandList()->Open(window.graphics);

		// drawing objects
		{
			triangle->Draw(window.graphics, pipeline);
		}

		// drawing imgui layer
		imguiLayer.Draw(window.graphics, pipeline);

		// closing graphics command list
		pipeline.GetGraphicCommandList()->Close(window.graphics);
	}

	// executing command lists
	pipeline.Execute(window.graphics);
	
	window.graphics.FinishFrame();

	window.graphics.WaitForGPU();
}