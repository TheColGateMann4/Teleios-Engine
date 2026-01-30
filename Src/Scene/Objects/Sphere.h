#pragma once
#include "Scene/SceneObject.h"

class Graphics;
class Pipeline;

class Sphere : public SceneObject
{
public:
	Sphere(Graphics& graphics, DirectX::XMFLOAT3 position = { 0.0f, 0.0f, 0.0f }, DirectX::XMFLOAT3 rotation = { 0.0f, 0.0f, 0.0f }, float diameter = 1.0f, size_t tesselation = 3);

public:
	virtual void Initialize(Graphics& graphics, Pipeline& pipeline) override;

	virtual void DrawAdditionalPropeties(Graphics& graphics, Pipeline& pipeline) override;

private:
	void UpdateMesh(Graphics& graphics, Pipeline& pipeline);

private:
	bool m_initialized;
	float m_diameter;
	size_t m_tesselation;
};