#pragma once
#include "Includes/CppIncludes.h"
#include "Window.h"

class Application
{
public:
	Application(unsigned int width, unsigned int height, const char* name);

	~Application();

public:
	int Run();

	void Update();

private:
	CONSOLE_HANDLE
	const char* m_name;

private:
	Window window;
};