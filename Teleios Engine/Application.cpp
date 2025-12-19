#include "Application.h"
#include "Macros/ErrorMacros.h"

#include "Scene/Objects/Cube.h"
#include "Scene/Objects/Model.h"
#include "Scene/Objects/PointLight.h"

Application::Application(unsigned int width, unsigned int height, const char* name)
	:
	m_name(name),
	window(width, height, m_name, DXGI_FORMAT_R16G16B16A16_FLOAT),
	scene(window.graphics)
{

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
	scene.BeginInitialization(window.graphics);


	scene.AddSceneObject(std::make_shared<Camera>(window.graphics));
	//scene.AddSceneObject(std::make_shared<Cube>(window.graphics));
	//scene.AddSceneObjectFromFile(window.graphics, "Models/nanosuit/nanosuit.obj");
	scene.AddSceneObjectFromFile(window.graphics, "Models/sponza/sponza.obj", 1.0f / 40.0f);
	//scene.AddSceneObjectFromFile(window.graphics, "Models/intel_sponza/main1_sponza/NewSponza_Main_Yup_003.fbx", 1.0f);
	//scene.AddSceneObjectFromFile(window.graphics, "Models/intel_sponza/pkg_a_curtains/NewSponza_Curtains_FBX_YUp.fbx", 1.0f);

	//scene.AddSceneObjectFromFile(window.graphics, "Models/intel_sponza/main1_sponza/NewSponza_Main_glTF_003.gltf");
	//scene.AddSceneObjectFromFile(window.graphics, "Models/intel_sponza/pkg_a_curtains/NewSponza_Curtains_glTF.gltf");
	//scene.AddSceneObjectFromFile(window.graphics, "Models/intel_sponza/pkg_b_ivy/NewSponza_IvyGrowth_glTF.gltf");
	//scene.AddSceneObjectFromFile(window.graphics, "Models/intel_sponza/pkg_c_trees/NewSponza_CypressTree_glTF.gltf");

	scene.AddSceneObject(std::make_shared<PointLight>(window.graphics, scene));

	scene.InitializeSceneObjects(window.graphics);


	scene.FinishInitialization(window.graphics);
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

	scene.DrawObjectInspector(window.graphics);

	scene.RenderImguiLayer(window, window.graphics);

	scene.UpdateSceneObjects(window, window.graphics);

	scene.DrawSceneObjects(window.graphics);
	

	window.graphics.FinishFrame();
}