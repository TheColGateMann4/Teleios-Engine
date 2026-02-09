#pragma once
#include "Includes/CppIncludes.h"
#include "Graphics/Core/Pipeline.h"
#include "Graphics/Imgui/ImguiLayer.h"

#include "Material.h"

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

	void AddMaterial(std::string name, std::shared_ptr<Material> material);

public:
	// starts initialization state in pipeline, allows copying GPU resources and performing GPU operations
	void BeginInitialization(Graphics& graphics);

	// ends initialization state in pipeline
	void FinishInitialization(Graphics& graphics);

	void InitializeCameraBuffer(Graphics& graphics, Pipeline& pipeline);

	void InitializeLightBuffer(Graphics& graphics, Pipeline& pipeline);

	void AssignJobs(Graphics& graphics);

	void InitializeSceneObjects(Graphics& graphics);

	// resources that need to be initialized using compute shaders, eg. mip maps for textures
	void UpdateGraphicResources(Graphics& graphics);

	void DrawObjectInspector(Graphics& graphics);

	void Update(Graphics& graphics, const Input& input, bool isCursorLocked);

	void UpdateBuffersIfNeeded(Graphics& graphics);

	std::vector<std::shared_ptr<SceneObject>>& GetObjects();

	const std::vector<PointLight*>& GetPointLights();

	Camera* GetCurrentCamera() const;

	void SetActiveCamera(Camera* camera);

	std::shared_ptr<Material> GetMaterial(const std::string& name);

private:
	unsigned int GetOriginalNameIndex(std::string name);

	void UpdateObjectMatrices(Graphics& graphics);

	void m_SetActiveCamera(Camera* camera);

private:

	using NameBucket = std::vector<SceneObject*>;
	std::unordered_map<std::string, NameBucket> m_nameRegistry;
	std::vector<std::shared_ptr<SceneObject>> m_sceneObjects = {};

	SceneObject* m_objectSelectedInHierarchy = nullptr;

	std::vector<Camera*> m_cameras;
	Camera* m_activeCamera = nullptr;

	std::vector<PointLight*> m_pointlights;
	std::shared_ptr<CachedConstantBuffer> m_lightBuffer;
	std::shared_ptr<CachedConstantBuffer> m_cameraBuffer;

	std::unordered_map<std::string, std::shared_ptr<Material>> m_materials;
};