#pragma once
#include "Includes/CppIncludes.h"
#include "Window.h"
#include "Triangle.h"
#include "Camera.h"

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

	std::shared_ptr<Triangle> triangle;
	std::shared_ptr<Camera> camera;
};