#pragma once
#include "Includes/CppIncludes.h"

class Application
{
public:
	Application(unsigned int width, unsigned int height, const char* name);

	~Application();

public:
	int Run();

private:
	CONSOLE_HANDLE
};