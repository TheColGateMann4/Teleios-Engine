#pragma once


class CPUProfiler
{
public:
	float GetWorkTime() const;
	float GetTotalTime() const;

	void SetBeginData(float deltaTime);
	void SetEndData(float deltaTime);

private:
	float m_worktime = 0.0f;
	float m_totaltime = 0.0f;
	unsigned int m_framesPassed = 0;
};