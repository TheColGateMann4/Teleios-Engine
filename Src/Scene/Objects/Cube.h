#pragma once
#include "Scene/SceneObject.h"
#include "Scene/Mesh.h"

class Graphics;
class Pipeline;

class Cube : public SceneObject
{
public:
	Cube(Graphics& graphics, DirectX::XMFLOAT3 position = {-2.0f, 0.0f, 0.0f}, DirectX::XMFLOAT3 rotation = { 0.0f, 0.0f, 0.0f });

private:
	CachedConstantBuffer* m_constantBuffer;
};