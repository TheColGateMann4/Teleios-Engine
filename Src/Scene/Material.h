#pragma once
#include "Includes/DirectXIncludes.h"
#include "Graphics/Bindables/Shader.h"

class Bindable;

namespace MaterialProperties
{
	struct MaterialProperties
	{
		bool hasAnyMap = false;

		bool hasAlbedoMap = false;
		bool hasNormalMap = false;
		bool hasSpecularMap = false;
		bool hasGlosinessMap = false;
		bool hasMetalnessMap = false;
		bool hasRoughnessMap = false;
		bool hasAmbientMap = false;

		bool metalRoughnessSystem = false;

		std::string albedoMapPath;
		std::string normalMapPath;
		std::string specularMetalnessMapPath;
		std::string glosinessRoughnessMapPath;
		std::string ambientMapPath;

		// colors
		DirectX::XMFLOAT3 albedo = { 0.0f, 0.0f, 0.0f };
		DirectX::XMFLOAT3 ambient = { 0.0f, 0.0f, 0.0f };
		DirectX::XMFLOAT3 specularColor = { 1.0f, 1.0f, 1.0f };
		DirectX::XMFLOAT3 reflective = { 0.03f, 0.03f, 0.03f };

		// texture specific settings
		bool specularOneChannelOnly = false;
		bool ignoreDiffseAlpha = false;
		bool twoSided = false;
		bool roughnessMetalnessInOneTexture = false;

		// specular/golsiness values
		float specular = 0.1f;
		float glosiness = 1.0f;
		float metalness = 0.0f;
		float roughness = 0.5f;
		float opacity = 1.0f;
	};
};

class Material
{
	friend class ModelImporter;

public:
	Material(Graphics& graphics, std::string filePath, MaterialProperties::MaterialProperties properties);

public:
	const MaterialProperties::MaterialProperties& GetProperties() const;
	std::vector<ShaderMacro> GetShaderMacros() const;
	const std::vector<std::shared_ptr<Bindable>>& GetBindables() const;

private:
	std::vector<std::shared_ptr<Bindable>> m_bindables;
	std::vector<ShaderMacro> m_shaderMacros;
	MaterialProperties::MaterialProperties m_properties = {};
};