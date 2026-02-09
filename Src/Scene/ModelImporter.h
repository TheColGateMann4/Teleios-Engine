#pragma once
#include "Includes/CppIncludes.h"
#include "Material.h"

class Graphics;
class Scene;
class Model;

struct aiScene;
class aiNode;
class aiMesh;
class aiMaterial;

class ModelImporter
{
public:
	static void AddSceneObjectFromFile(Graphics& graphics, const char* path, float scale, Scene& scene);

private:
	static void ProcessLights(Graphics& graphics, Scene& scene, const aiScene& importedScene);
	static void ProcessCameras(Graphics& graphics, Scene& scene, const aiScene& importedScene);
	static void ProcessMaterials(Graphics& graphics, Scene& scene, const aiScene& importedScene, const std::string& filePath);
	static void PushModel(Graphics& graphics, Scene& scene, float scale, aiNode* node, aiMesh** meshes, aiMaterial** materials, Model* pParent = nullptr);

private:
	static std::shared_ptr<Material> ProcessMaterial(Graphics& graphics, aiMaterial* material, const std::string& filePath);
	static MaterialProperties::MaterialProperties ProcessMaterialProperties(aiMaterial* material);
};