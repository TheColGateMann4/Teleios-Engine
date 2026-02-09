#pragma once
#include "Includes/CppIncludes.h"
#include "Scene/SceneObject.h"

class aiNode;
class aiMesh;

class Material;

class Graphics;

class Model : public SceneObject
{
public:
	Model(Graphics& graphics, Model* pParent, aiNode* node, std::vector<std::pair<aiMesh*, std::shared_ptr<Material>>> modelMeshes, float scale = 1.0f, DirectX::XMFLOAT3 position = { 0.0f, 0.0f, 0.0f });

	Model(const Model&) = delete;

	Model(Model&&) = delete;
};

