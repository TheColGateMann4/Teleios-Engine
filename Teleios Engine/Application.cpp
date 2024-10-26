#include "Application.h"
#include "Macros/ErrorMacros.h"
#include "Triangle.h"

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
	sceneObjects.push_back(std::make_shared<Triangle>(window.graphics, pipeline));

	for (auto& sceneObject : sceneObjects)
		sceneObject->Initialize(window.graphics);
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

		for (auto& sceneObject : sceneObjects)
			sceneObject->DrawImguiWindow(window.graphics, imguiLayer.IsVisible());

		camera->DrawImguiWindow(imguiLayer.IsVisible());

		pointLight->DrawImguiWindow(window.graphics, imguiLayer.IsVisible());

		imguiLayer.DrawDemoWindow();
	}

	// making imgui get its accumulated commands and heaps ready
	imguiLayer.Render();

	// updating objects
	{
		camera->Update(window.input, window.GetCursorLocked());

		for (auto& sceneObject : sceneObjects)
			sceneObject->UpdateTransformMatrix(window.graphics, *camera);

		for (auto& sceneObject : sceneObjects)
			sceneObject->Update(window.graphics);
	}

	// rendering 
	{
		
		CommandList* commandList = pipeline.GetGraphicCommandList(); 
		commandList->Open(window.graphics);	// opening graphics command list and clearning allocator

		// setting render target
		commandList->SetRenderTarget(window.graphics, window.graphics.GetBackBuffer(), window.graphics.GetDepthStencil());

		commandList->SetResourceState(window.graphics, window.graphics.GetBackBuffer(), D3D12_RESOURCE_STATE_RENDER_TARGET); // setting backbuffer state to renderTarget on drawing time

		{
			D3D12_VIEWPORT viewport = {};
			viewport.TopLeftX = 0;
			viewport.TopLeftY = 0;
			viewport.Width = window.graphics.GetWidth();
			viewport.Height = window.graphics.GetHeight();
			viewport.MinDepth = 0.0f;
			viewport.MaxDepth = 1.0f;

			commandList->Get()->RSSetViewports(1, &viewport); // setting viewports

			D3D12_RECT viewportRect = {};
			viewportRect.left = viewportRect.top = 0;
			viewportRect.bottom = window.graphics.GetHeight();
			viewportRect.right = window.graphics.GetWidth();

			commandList->Get()->RSSetScissorRects(1, &viewportRect); // setting scissor rects
		}

		commandList->ClearRenderTargetView(window.graphics, window.graphics.GetBackBuffer()); // clearning render target from previous frames
		commandList->ClearDepthStencilView(window.graphics, window.graphics.GetDepthStencil()); // clearning depth stencil from previous frames

		// drawing scene objects
		for (auto& sceneObject : sceneObjects)
			sceneObject->Draw(window.graphics, pipeline);

		// drawing imgui layer
		imguiLayer.Draw(window.graphics, pipeline);

		commandList->SetResourceState(window.graphics, window.graphics.GetBackBuffer(), D3D12_RESOURCE_STATE_PRESENT); // setting backbuffer state to present since we finished drawing
		commandList->Close(window.graphics); // closing graphics command list
	}

	// executing command lists
	pipeline.Execute(window.graphics);
	
	window.graphics.FinishFrame();

	window.graphics.WaitForGPU();
}