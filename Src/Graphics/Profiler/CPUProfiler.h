#pragma once


class CPUProfiler
{
public:
	float GetData() const;

	void SetBeginData(float deltaTime);
	void EndBeginData(float deltaTime);

private:
	float m_data = 0.0f;
	unsigned int m_framesPassed = 0;
};