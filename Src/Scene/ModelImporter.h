#pragma once
#include "Includes/CppIncludes.h"

class Graphics;
class Scene;
class Model;

class aiNode;
class aiMesh;
class aiMaterial;

class ModelImporter
{
public:
	static void AddSceneObjectFromFile(Graphics& graphics, const char* path, float scale, Scene& scene);

private:
	static void PushModel(Graphics& graphics, Scene& scene, float scale, std::string& filePath, aiNode* node, aiMesh** meshes, aiMaterial** materials, Model* pParent = nullptr);
};