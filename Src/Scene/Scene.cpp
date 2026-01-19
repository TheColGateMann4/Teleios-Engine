#include "Scene.h"
#include "Graphics/Core/Graphics.h"
#include "System/Window.h"
#include "SceneObject.h"
#include "Objects/Camera.h"
#include "Objects/Model.h"

#include "ModelImporter.h"

#include <imgui.h>

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

void Scene::BeginInitialization(Graphics& graphics)
{
	// for now we will use graphic command list for simplicity
	graphics.GetRenderer().GetPipeline().GetGraphicCommandList()->Open(graphics);
}

void Scene::FinishInitialization(Graphics& graphics)
{
	Renderer& renderer = graphics.GetRenderer();
	Pipeline& pipeline = renderer.GetPipeline();

	InitializeSceneObjects(graphics);

	AssignJobs(graphics);

	renderer.GatherJobBindables();

	renderer.InitializeJobs(graphics);

	renderer.InitializePasses(graphics);

	renderer.FinishInitialization(graphics);

	graphics.WaitForGPU();
}

void Scene::AssignJobs(Graphics& graphics)
{
	for (auto& sceneObject : m_sceneObjects)
		sceneObject->SubmitJobs(graphics.GetRenderer());

	graphics.GetRenderer().SubmitPassesJobs();

	graphics.GetRenderer().AssignJobsToPasses();
}

void Scene::InitializeSceneObjects(Graphics& graphics)
{
	Pipeline& pipeline = graphics.GetRenderer().GetPipeline();

	// adding static resources to scene first
	for (auto& sceneObject : m_sceneObjects)
		sceneObject->InternalAddStaticResources(pipeline);

	// after every object was firstly initialized, we create descriptor heap with space for each one
	graphics.GetDescriptorHeap().Finish(graphics);

	// we are creating one big constant buffer that will hold every constant buffer on scene
	graphics.GetConstantBufferHeap().Finish(graphics);

	// after we created descriptor heap we are making objects use this to make SRV's
	for (auto& sceneObject : m_sceneObjects)
		sceneObject->InternalInitialize(graphics, pipeline);
}

void Scene::UpdateGraphicResources(Graphics& graphics)
{
	Pipeline& pipeline = graphics.GetRenderer().GetPipeline();

	// copying constant buffers to GPU
	graphics.GetConstantBufferHeap().CopyResources(graphics, pipeline.GetGraphicCommandList());
}

void Scene::DrawObjectInspector(Graphics& graphics)
{
	if (!graphics.GetRenderer().GetImguiLayer().IsVisible())
		return;

	if(ImGui::Begin("Scene Inspector"))
	{
		if(ImGui::BeginTable("SceneInspectorTable", 2, ImGuiTableFlags_Resizable | ImGuiTableFlags_BordersInnerV))
		{
			ImGui::TableNextColumn();
			ImGui::BeginChild("Hierarchy", ImVec2(0, 0), false);
			{
				for (auto& sceneObject : m_sceneObjects)
					if (!sceneObject->isChild())
						sceneObject->DrawHierarchy(&m_objectSelectedInHierarchy);
			}
			ImGui::EndChild();

			ImGui::TableNextColumn();
			ImGui::BeginChild("Inspector", ImVec2(0, 0), false);
			{
				if (m_objectSelectedInHierarchy != nullptr)
				{
					m_objectSelectedInHierarchy->DrawTransformPropeties();

					ImGui::NewLine();

					m_objectSelectedInHierarchy->DrawAdditionalPropeties(graphics, graphics.GetRenderer().GetPipeline());

					ImGui::NewLine();

					m_objectSelectedInHierarchy->DrawConstantBuffers(graphics);
				}
			}
			ImGui::EndChild();

			ImGui::EndTable();
		}
	}

	ImGui::End();
}

void Scene::Update(Graphics& graphics, const Input& input, bool isCursorLocked)
{
	m_camera->UpdateCamera(input, isCursorLocked); // it is important that active camera gets updated before other objects, since for example pointlight checks if position or rotation was updated to determine if constant buffer should be updated

	for (auto& sceneObject : m_sceneObjects)
		sceneObject->InternalUpdate(graphics, *m_camera, graphics.GetRenderer().GetPipeline());

	graphics.GetConstantBufferHeap().UpdateHeap(graphics);

	UpdateObjectMatrices(graphics);

	UpdateGraphicResources(graphics);
}

std::vector<std::shared_ptr<SceneObject>>& Scene::GetObjects()
{
	return m_sceneObjects;
}

std::string Scene::GetOriginalName(std::string name)
{
	return name; // temporary
}

void Scene::UpdateObjectMatrices(Graphics& graphics)
{
	// calculating transform matrix for each object that needs it
	for (auto& sceneObject : m_sceneObjects)
		sceneObject->UpdateLocalTransformIfNeeded();

	// passing calculated matrix down the object hierarchy
	for (auto& sceneObject : m_sceneObjects)
		if (!sceneObject->isChild())
			sceneObject->UpdateParentMatrix();

	// after all matrices are set up, we send them to camera
	for (auto& sceneObject : m_sceneObjects)
		sceneObject->UpdateTransformBufferIfNeeded(graphics, *graphics.GetRenderer().GetPipeline().GetCurrentCamera());
}