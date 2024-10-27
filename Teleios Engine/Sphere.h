#pragma once
#include "SceneObject.h"

class Sphere : public SceneObject
{
public:
	Sphere(Graphics& graphics, DirectX::XMFLOAT3 position = { 0.0f, 0.0f, 0.0f }, DirectX::XMFLOAT3 rotation = { 0.0f, 0.0f, 0.0f });


private:
	void UpdateMesh(Graphics& graphics);
};

