#pragma once
#include "Includes/CppIncludes.h"

class Time
{
public:
	Time();

public: // timer capabilities
	float Mark();
	float Peek() const;

public: // global time
	float GetElapsed() const;

private:
	std::chrono::time_point<std::chrono::steady_clock> startTime;
	std::chrono::time_point<std::chrono::steady_clock> markTime;
};