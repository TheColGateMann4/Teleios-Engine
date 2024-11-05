#pragma once
#include "includes/CppIncludes.h"
#include "SceneObject.h"
#include "ModelMesh.h"

class Graphics;

class Model : public SceneObject
{
public:
	Model(Graphics& graphics, const char* path, float scale = 1.0f);

private:
	std::vector<std::shared_ptr<ModelMesh>> m_meshes;
};

