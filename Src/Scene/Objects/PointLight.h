#pragma once
#include "Scene/SceneObject.h"

#include "includes/CppIncludes.h"
#include "Bindables/ConstantBuffer.h"
#include "Sphere.h"

class Graphics;
class Scene;
class Pipeline;

class PointLight : public SceneObject
{
public:
	PointLight(Graphics& graphics, Scene& scene, DirectX::XMFLOAT3 position = {-1.5f, 1.0f, -1.5f}, DirectX::XMFLOAT3 color = { 1.0f, 1.0f, 1.0f });

public:
	virtual void AddStaticResources(Pipeline& pipeline) override;
	
	virtual void Update(Graphics& graphics, Pipeline& pipeline) override;

	virtual void DrawTransformPropeties() override;

	virtual void DrawAdditionalPropeties(Graphics& graphics, Pipeline& pipeline) override;

private:
	DirectX::XMFLOAT3 m_position;
	DirectX::XMFLOAT3 m_color;
	
	std::shared_ptr<CachedConstantBuffer> m_lightBuffer;
	bool m_transformChanged;
	Sphere* m_pSphereModel;
};

