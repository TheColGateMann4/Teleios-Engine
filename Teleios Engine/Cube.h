#pragma once
#include "SceneObject.h"
#include "Drawable.h"

class Graphics;
class Pipeline;

class Cube : public SceneObject, public Drawable
{
public:
	Cube(Graphics& graphics, DirectX::XMFLOAT3 position = {-2.0f, 0.0f, 0.0f}, DirectX::XMFLOAT3 rotation = { 0.0f, 0.0f, 0.0f });

public:
	virtual void DrawImguiWindow(Graphics& graphics, bool isLayerVisible) override;

private:
	CachedConstantBuffer* m_constantBuffer;
};