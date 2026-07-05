#pragma once
#include "Includes/DirectXIncludes.h"
#include "Graphics/Bindables/Shader.h"
#include "Graphics/Core/BindableContainer.h"
#include "Graphics/Bindables/RasterizerState.h"

class Bindable;

namespace MaterialProperties
{
	enum class MaterialWorkflow
	{
		none,
		metalnessRoughness,
		specularGlossiness
	};

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

		MaterialWorkflow materialWorkflow = MaterialWorkflow::metalnessRoughness;

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
	const MeshBindableContainer& GetBindableContainer() const;

	void Bind(Graphics& graphics, CommandList* commandList);

	void Initialize(Graphics& graphics);

	void InitializeGraphicResources(Graphics& graphics, Pipeline& pipeline);

	void Update();

	D3D12_GPU_DESCRIPTOR_HANDLE GetDescriptorHeapGPUHandle(Graphics& graphics) const;

	ObjectRasterizerStateOptions GetRasterizerOptions() const;

private:
	MeshBindableContainer m_bindableContainer;
	MaterialProperties::MaterialProperties m_properties = {};

	ObjectRasterizerStateOptions m_rasterizerOptions = {};
};