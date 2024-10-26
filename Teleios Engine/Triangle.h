#pragma once
#include "SceneObject.h"

class Graphics;
class Pipeline;
class Camera;

class Triangle : public SceneObject
{
public:
	Triangle(Graphics& graphics, Pipeline& pipeline, DirectX::XMFLOAT3 position = {0.0f, 0.0f, 0.0f}, DirectX::XMFLOAT3 rotation = { 0.0f, 0.0f, 0.0f });

public:
	virtual void DrawImguiWindow(Graphics& graphics, bool isLayerVisible) override;

private:
	CachedConstantBuffer* m_constantBuffer;
};