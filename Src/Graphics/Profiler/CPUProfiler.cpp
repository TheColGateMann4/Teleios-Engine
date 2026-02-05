#include "CPUProfiler.h"

float CPUProfiler::GetData() const
{
	return m_data;
}

void CPUProfiler::SetBeginData(float deltaTime)
{
	m_data = deltaTime;
}
void CPUProfiler::EndBeginData(float deltaTime)
{
	m_data -= deltaTime;
}