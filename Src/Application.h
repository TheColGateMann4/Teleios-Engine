#pragma once
#include "Includes/CppIncludes.h"
#include "System/Window.h"
#include "Scene/SceneObject.h"
#include "Scene/Objects/Camera.h"
#include "Scene/Scene.h"

class Application
{
public:
	Application(const Application&) = delete;

	Application(unsigned int width, unsigned int height, const char* name);

	~Application();

public:
	int Run();

	void InitializeScene();

	void Update();

private:
	const char* m_name;

private:
	Window window;
	Scene scene;

	std::vector<std::shared_ptr<SceneObject>> sceneObjects;
	std::shared_ptr<Camera> camera;
};