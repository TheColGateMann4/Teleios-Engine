#include "Scene.h"
#include "Graphics.h"
#include "Window.h"
#include "SceneObject.h"
#include "Camera.h"
#include "Model.h"

#include "ModelImporter.h"

Scene::Scene(Graphics& graphics)
	:
	m_pipeline(graphics),
	m_imguiLayer(graphics)
{

}

void Scene::AddSceneObjectFromFile(Graphics& graphics, const char* path, float scale)
{
	ModelImporter::AddSceneObjectFromFile(graphics, path, scale, *this);
}

void Scene::AddSceneObjectFromFile(std::shared_ptr<Model> model, std::string objectName)
{
	model->SetName(GetOriginalName(objectName));

	m_sceneObjects.push_back(model);
}

void Scene::AddSceneObject(std::shared_ptr<SceneObject> sceneObject)
{
	if (auto camera = dynamic_cast<Camera*>(sceneObject.get()))
		m_camera = camera;

	m_sceneObjects.push_back(sceneObject);
}

void Scene::InitializeSceneObjects(Graphics& graphics)
{
	// adding static resources to scene first
	for (auto& sceneObject : m_sceneObjects)
		sceneObject->AddStaticResources(m_pipeline);

	// after every object was firstly initialized, we create descriptor heap with space for each one
	graphics.GetDescriptorHeap().Finish(graphics);

	// we are creating one big constant buffer that will hold every constant buffer on scene
	graphics.GetConstantBufferHeap().Finish(graphics);

	// after we created descriptor heap we are making objects use this to make SRV's
	for (auto& sceneObject : m_sceneObjects)
		sceneObject->InternalInitialize(graphics, m_pipeline);
}

void Scene::InitializeGraphicResources()
{

}

void Scene::RenderImguiLayer(Window& window, Graphics& graphics)
{
	// initializing imgui windows
	{
		window.input.DrawImguiWindow(m_imguiLayer.IsVisible());

		for (auto& sceneObject : m_sceneObjects)
			sceneObject->DrawImguiWindow(graphics, m_imguiLayer.IsVisible());

		m_imguiLayer.DrawDemoWindow();
	}

	// making imgui get its accumulated commands and heaps ready
	m_imguiLayer.Render();
}

void Scene::UpdateSceneObjects(Window& window, Graphics& graphics)
{
	m_camera->UpdateCamera(window.input, window.GetCursorLocked()); // it is important that active camera gets updated before other objects, since for example pointlight checks if position or rotation was updated to determine if constant buffer should be updated

	for (auto& sceneObject : m_sceneObjects)
		sceneObject->InternalUpdate(graphics, *m_camera, m_pipeline);

	graphics.GetConstantBufferHeap().UpdateHeap(graphics);
}

void Scene::DrawSceneObjects(Graphics& graphics)
{
	{
		m_pipeline.BeginRender(graphics);

		graphics.GetConstantBufferHeap().CopyResources(graphics, m_pipeline.GetGraphicCommandList());

		// drawing scene objects
		for (auto& sceneObject : m_sceneObjects)
			sceneObject->InternalDraw(graphics, m_pipeline);

		// drawing imgui layer
		if(m_imguiLayer.IsVisible())
		m_imguiLayer.Draw(graphics, m_pipeline);

		m_pipeline.FinishRender(graphics);
	}

	// executing command lists
	m_pipeline.Execute(graphics);
}

ImguiLayer& Scene::GetImguiLayer()
{
	return m_imguiLayer;
}