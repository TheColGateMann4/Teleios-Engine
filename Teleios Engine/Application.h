#pragma once
#include "Includes/CppIncludes.h"
#include "Window.h"
#include "SceneObject.h"
#include "Camera.h"
#include "Scene.h"

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