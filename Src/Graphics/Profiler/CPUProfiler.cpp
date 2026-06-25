#include "CPUProfiler.h"

float CPUProfiler::GetWorkTime() const
{
	return m_worktime;
}

float CPUProfiler::GetTotalTime() const
{
	return m_totaltime;
}

void CPUProfiler::SetBeginData(float deltaTime)
{
	m_totaltime = deltaTime; // when first deltatime is whole frame + wait for vsync
}

void CPUProfiler::SetEndData(float deltaTime)
{
	m_worktime = deltaTime; // second deltatime is only work time without vsync wait
}