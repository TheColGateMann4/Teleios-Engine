#pragma once
#include "Includes/CppIncludes.h"
#include "Window.h"
#include "Pipeline.h"
#include "ImguiLayer.h"
#include "Triangle.h"
#include "Camera.h"
#include "PointLight.h"

class Application
{
public:
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
	Pipeline pipeline;
	ImguiLayer imguiLayer;

	std::shared_ptr<Triangle> triangle;
	std::shared_ptr<Camera> camera;
	std::shared_ptr<PointLight> pointLight;
};