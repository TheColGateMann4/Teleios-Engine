#pragma once
#include "includes/CppIncludes.h"
#include "Pipeline.h"
#include "ImguiLayer.h"

class Window;
class Graphics;
class SceneObject;
class Camera;

class Scene
{
public:
	Scene(Graphics& graphics);

public:
	void AddSceneObject(std::shared_ptr<SceneObject> sceneObject);

public:
	void InitializeSceneObjects(Graphics& graphics);

	// resources that need to be initialized using compute shaders, eg. mip maps for textures
	void InitializeGraphicResources();

	void RenderImguiLayer(Window& window, Graphics& graphics);

	void UpdateSceneObjects(Window& window, Graphics& graphics);

	void DrawSceneObjects(Graphics& graphics);

public:
	ImguiLayer& GetImguiLayer();

private:
	Pipeline m_pipeline;
	ImguiLayer m_imguiLayer;

	std::vector<std::shared_ptr<SceneObject>> m_sceneObjects = {};
	Camera* m_camera = nullptr;
};