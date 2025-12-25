#pragma once
#include "Includes/CppIncludes.h"
#include "System/Time.h"
#include "System/Window.h"
#include "Scene/SceneObject.h"
#include "Scene/Objects/Camera.h"
#include "Scene/Scene.h"
#include "Graphics/Core/Pipeline.h"

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
	Time time;
	Window window;
	Graphics graphics;
	Scene scene;
};