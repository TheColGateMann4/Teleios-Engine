#include "Application.h"
#include "Macros/ErrorMacros.h"

#include "Scene/Objects/Cube.h"
#include "Scene/Objects/Model.h"
#include "Scene/Objects/PointLight.h"

Application::Application(unsigned int width, unsigned int height, const char* name)
	:
	m_name(name),
	window(width, height, m_name),
	graphics(window.GetHWnd(), DXGI_FORMAT_R16G16B16A16_FLOAT),
	scene()
{
	window.SetFunctionCallback(graphics.GetImguiManager()->HandleMessages);
};

Application::~Application()
{

}

int Application::Run()
{
	InitializeScene();

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
	scene.BeginInitialization(graphics);


	scene.AddSceneObject(std::make_shared<Camera>(graphics));
	scene.AddSceneObject(std::make_shared<Camera>(graphics));
	scene.AddSceneObject(std::make_shared<Camera>(graphics));
	//scene.AddSceneObject(std::make_shared<Cube>(graphics));
	//scene.AddSceneObjectFromFile(graphics, "Models/nanosuit/nanosuit.obj");
	scene.AddSceneObjectFromFile(graphics, "Models/sponza/sponza.obj", 1.0f / 40.0f);
	//scene.AddSceneObjectFromFile(graphics, "Models/intel_sponza/main1_sponza/NewSponza_Main_Yup_003.fbx", 1.0f);
	//scene.AddSceneObjectFromFile(graphics, "Models/intel_sponza/pkg_a_curtains/NewSponza_Curtains_FBX_YUp.fbx", 1.0f);

	//scene.AddSceneObjectFromFile(graphics, "Models/intel_sponza/main1_sponza/NewSponza_Main_glTF_003.gltf");
	//scene.AddSceneObjectFromFile(graphics, "Models/intel_sponza/pkg_a_curtains/NewSponza_Curtains_glTF.gltf");
	//scene.AddSceneObjectFromFile(graphics, "Models/intel_sponza/pkg_b_ivy/NewSponza_IvyGrowth_glTF.gltf");
	//scene.AddSceneObjectFromFile(graphics, "Models/intel_sponza/pkg_c_trees/NewSponza_CypressTree_glTF.gltf");

	scene.AddSceneObject(std::make_shared<PointLight>(graphics, scene));

	scene.FinishInitialization(graphics);
}

void Application::Update()
{
	graphics.BeginFrame();

	ImguiLayer& imguiLayer = graphics.GetRenderer().GetImguiLayer();


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

	scene.DrawObjectInspector(graphics);

	window.input.DrawImguiWindow(imguiLayer.IsVisible());
	graphics.GetRenderer().DrawImguiWindow(graphics);
	imguiLayer.DrawDemoWindow();

	scene.Update(graphics, window.input, window.GetCursorLocked());

	graphics.Render(scene);

	graphics.FinishFrame();
}