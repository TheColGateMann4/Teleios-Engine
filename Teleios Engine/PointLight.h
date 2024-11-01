#pragma once
#include "SceneObject.h"

#include "includes/CppIncludes.h"
#include "ConstantBuffer.h"
#include "Sphere.h"

class Pipeline;

class PointLight : public SceneObject
{
public:
	PointLight(Graphics& graphics, Pipeline& pipeline, DirectX::XMFLOAT3 position = {-1.5f, 0.0f, -1.5f}, DirectX::XMFLOAT3 color = { 1.0f, 1.0f, 1.0f });

public:
	virtual void Update(Graphics& graphics, Pipeline& pipeline) override;

	virtual void DrawImguiWindow(Graphics& graphics, bool isLayerVisible) override;

private:
	DirectX::XMFLOAT3 m_position;
	DirectX::XMFLOAT3 m_color;
	
	std::shared_ptr<CachedConstantBuffer> m_lightBuffer;
	Sphere m_model;
	bool m_transformChanged;
};

