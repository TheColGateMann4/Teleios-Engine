#pragma once
#include "Scene/SceneObject.h"

#include "Includes/CppIncludes.h"
#include "Graphics/Bindables/ConstantBuffer.h"
#include "Sphere.h"

class Graphics;
class Scene;
class Pipeline;

class PointLight : public SceneObject
{
public:
	PointLight(Graphics& graphics, Scene& scene, DirectX::XMFLOAT3 position = {-1.5f, 1.0f, -1.5f}, DirectX::XMFLOAT3 color = { 1.0f, 1.0f, 1.0f });

public:
	virtual void Initialize(Graphics& graphics, Pipeline& pipeline) override;
	
	virtual void Update(Graphics& graphics, Pipeline& pipeline) override;

	void UpdateLight(Graphics& graphics, Scene& scene);

	virtual void DrawTransformPropeties(Scene& scene) override;

	virtual void DrawAdditionalPropeties(Graphics& graphics, Pipeline& pipeline) override;

	virtual SceneObjectType GetSceneObjectType() override;

	void SetLightIndex(unsigned int lightIndex);

private:
	DirectX::XMFLOAT3 m_position;
	DirectX::XMFLOAT3 m_color;
	
	bool m_transformChanged;
	Sphere* m_pSphereModel;

	CachedConstantBuffer* m_pLightBuffer = nullptr;
	unsigned int m_lightIndex = -1;
};

