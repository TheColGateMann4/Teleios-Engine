#pragma once
#include "SceneObject.h"
#include "Drawable.h"

class Graphics;
class Pipeline;

class Sphere : public SceneObject, public Drawable
{
public:
	Sphere(Graphics& graphics, DirectX::XMFLOAT3 position = { 0.0f, 0.0f, 0.0f }, DirectX::XMFLOAT3 rotation = { 0.0f, 0.0f, 0.0f }, float diameter = 1.0f, size_t tesselation = 3);

public:
	virtual void DrawImguiWindow(Graphics& graphics, bool isLayerVisible) override;

private:
	void UpdateMesh(Graphics& graphics);

private:
	bool m_initialized;
	float m_diameter;
	size_t m_tesselation;
};