#pragma once
#include "Includes/CppIncludes.h"
#include "Graphics/Core/Pipeline.h"
#include "Graphics/Imgui/ImguiLayer.h"

class Input;
class Graphics;
class SceneObject;
class Camera;
class PointLight;
class Model;

class Scene
{
public:
	void AddSceneObjectFromFile(Graphics& graphics, const char* path, float scale = 1.0f);

	void AddSceneObject(std::shared_ptr<SceneObject> sceneObject);

	// called only by ModelImporter
	void AddSceneObjectFromFile(std::shared_ptr<Model> model, std::string objectName);

public:
	// starts initialization state in pipeline, allows copying GPU resources and performing GPU operations
	void BeginInitialization(Graphics& graphics);

	// ends initialization state in pipeline
	void FinishInitialization(Graphics& graphics);

	void AssignJobs(Graphics& graphics);

	void InitializeSceneObjects(Graphics& graphics);

	// resources that need to be initialized using compute shaders, eg. mip maps for textures
	void UpdateGraphicResources(Graphics& graphics);

	void DrawObjectInspector(Graphics& graphics);

	void Update(Graphics& graphics, const Input& input, bool isCursorLocked);

	std::vector<std::shared_ptr<SceneObject>>& GetObjects();

	Camera* GetCurrentCamera() const;

	void SetActiveCamera(Camera* camera);

private:
	unsigned int GetOriginalNameIndex(std::string name);

	void UpdateObjectMatrices(Graphics& graphics);

	void m_SetActiveCamera(Camera* camera);

private:

	std::vector<std::shared_ptr<SceneObject>> m_sceneObjects = {};
	std::vector<Camera*> m_cameras;
	Camera* m_activeCamera = nullptr;

	std::vector<PointLight*> m_pointlights;

	using NameBucket = std::vector<SceneObject*>;
	std::unordered_map<std::string, NameBucket> m_nameRegistry;

	SceneObject* m_objectSelectedInHierarchy = nullptr;
};