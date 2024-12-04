#pragma once
#include "includes/CppIncludes.h"
#include "SceneObject.h"

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

		std::string diffuseMapPath;
		std::string normalMapPath;
		std::string specularMapPath;

		// colors
		DirectX::XMFLOAT3 ambientColor = { 0.0f, 0.0f, 0.0f };
		DirectX::XMFLOAT3 diffuseColor = { 0.0f, 0.0f, 0.0f };
		DirectX::XMFLOAT3 specularColor = { 1.0f, 1.0f, 1.0f };

		// texture specific settings
		bool specularOneChannelOnly = false;
		bool ignoreDiffseAlpha = false;

		// phong specular varibles
		float specularShinnynes = 0.1f;
		float specularPower = 1.0f;
	};

public:
	Model(Graphics& graphics, aiMesh* mesh, aiMaterial* material, std::string filePath, float scale = 1.0f, DirectX::XMFLOAT3 position = { -2.0f, 0.0f, 0.0f }, DirectX::XMFLOAT3 rotation = { 0.0f, 0.0f, 0.0f });

	Model(const Model&) = delete;

	Model(Model&&) = delete;

private:
	MaterialPropeties ProcessMaterialPropeties(aiMaterial* material);
};

