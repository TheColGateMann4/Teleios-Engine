#include "Time.h"


Time::Time()
{
	startTime = std::chrono::steady_clock::now();
	markTime = std::chrono::steady_clock::now();
}

float Time::Mark()
{
	auto now = std::chrono::steady_clock::now();
	std::chrono::duration<float> delta = now - markTime;

	markTime = now;

	return delta.count();
}

float Time::Peek() const
{
	auto now = std::chrono::steady_clock::now();
	std::chrono::duration<float> delta = now - markTime;

	return delta.count();
}

float Time::GetElapsed() const
{
	auto now = std::chrono::steady_clock::now();
	std::chrono::duration<float> elapsed = now - startTime;

	return elapsed.count();
}