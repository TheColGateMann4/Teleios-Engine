#include "Application.h"
#include "Macros/ErrorMacros.h"

#include "Cube.h"
#include "Model.h"
#include "PointLight.h"

Application::Application(unsigned int width, unsigned int height, const char* name)
	:
	m_name(name),
	window(width, height, m_name, DXGI_FORMAT_R16G16B16A16_FLOAT),
	scene(window.graphics)
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
	scene.AddSceneObject(std::make_shared<Camera>(window.graphics));
	//scene.AddSceneObject(std::make_shared<Cube>(window.graphics));
	//scene.AddSceneObject(std::make_shared<Model>(window.graphics, "Models/nanosuit/nanosuit.obj"));
	scene.AddSceneObjectFromFile(window.graphics, "Models/sponza/sponza.obj", 1.0f / 40.0f);
	scene.AddSceneObject(std::make_shared<PointLight>(window.graphics));

	scene.InitializeSceneObjects(window.graphics);
}

void Application::Update()
{
	window.graphics.BeginFrame();

	ImguiLayer& imguiLayer = scene.GetImguiLayer();


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

	scene.RenderImguiLayer(window, window.graphics);

	scene.UpdateSceneObjects(window, window.graphics);

	scene.DrawSceneObjects(window.graphics);
	

	window.graphics.FinishFrame();

	window.graphics.WaitForGPUIfNeeded();

	window.graphics.CleanupResources();
}