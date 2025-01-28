#pragma once
#include "includes/CppIncludes.h"
#include "SceneObject.h"

class aiNode;
class aiMesh;
class aiMaterial;

class Graphics;

class Model : public SceneObject
{
public:
	struct MaterialPropeties
	{
		bool hasAnyMap = false;

		bool hasDiffuseMap = false;
		bool hasNormalMap = false;
		bool hasSpecularMap = false;
		bool hasGlosinessMap = false;
		bool hasMetalnessMap = false;
		bool hasRoughnessMap = false;
		bool hasAmbientMap = false;

		bool metalRoughnessSystem = false;

		std::string diffuseMapPath;
		std::string normalMapPath;
		std::string specularMetalnessMapPath;
		std::string glosinessRoughnessMapPath;
		std::string ambientMapPath;

		// colors
		DirectX::XMFLOAT3 ambient = { 0.0f, 0.0f, 0.0f };
		DirectX::XMFLOAT3 diffuse = { 0.0f, 0.0f, 0.0f };
		DirectX::XMFLOAT3 specularColor = { 1.0f, 1.0f, 1.0f };
		DirectX::XMFLOAT3 reflective = { 0.03f, 0.03f, 0.03f };

		// texture specific settings
		bool specularOneChannelOnly = false;
		bool ignoreDiffseAlpha = false;
		bool twoSided = false;

		// specular/golsiness values
		float specular = 0.1f;
		float glosiness = 1.0f;
		float metalness = 0.0f;
		float roughness = 0.5f;
		float opacity = 1.0f;
	};

public:
	Model(Graphics& graphics, Model* pParent, aiNode* node, std::vector<std::pair<aiMesh*, aiMaterial*>> modelMeshes, std::string filePath, float scale = 1.0f, DirectX::XMFLOAT3 position = { 0.0f, 0.0f, 0.0f });

	Model(const Model&) = delete;

	Model(Model&&) = delete;

private:
	MaterialPropeties ProcessMaterialPropeties(aiMaterial* material);
};

