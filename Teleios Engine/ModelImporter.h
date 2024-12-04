#pragma once
#include "includes/CppIncludes.h"

class Graphics;
class Scene;

class ModelImporter
{
public:
	static void AddSceneObjectFromFile(Graphics& graphics, const char* path, float scale, Scene& scene);
};