#pragma once
#include "Drawable.h"

class Graphics;

class aiMesh;
class aiMaterial;

class ModelMesh : public Drawable
{
public:
	ModelMesh(Graphics& graphics, aiMesh* mesh, aiMaterial* material, DirectX::XMFLOAT3 position = { -2.0f, 0.0f, 0.0f }, DirectX::XMFLOAT3 rotation = { 0.0f, 0.0f, 0.0f });

};

