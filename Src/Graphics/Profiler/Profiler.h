#pragma once
#include "CPUProfiler.h"
#include "GPUProfiler.h"

class CommandList;
class Graphics;

class Profiler
{
	template<class T>
	struct SmoothedData
	{
		T value;
		bool initialized = false;
	};

public:
	void Initialize(Graphics& graphics);

public:
	void Draw();

	void UpdateData();

	void SetBeginData(Graphics& graphics, CommandList* commandList, float deltaTime);
	void SetEndData(Graphics& graphics, CommandList* commandList, float deltaTime);

	template<class T>
	void SmoothData(SmoothedData<T>& val, T newVal)
	{
		if (!val.initialized)
		{
			val.value = newVal;
			val.initialized = true;
			return;
		}
		
		T delta = std::abs(newVal - val.value);
		T smooth = std::clamp<T>(delta / T(0.1), T(0.005f), T(0.2));

		val.value = std::lerp<T>(val.value, newVal, smooth);
	}

private:
	CPUProfiler m_cpuProfiler;
	GPUProfiler m_gpuProfiler;

	SmoothedData<float> m_fpsSmoothed = SmoothedData(0.0f);
	SmoothedData<float> m_cpuSmoothedData = SmoothedData(0.0f);	// seconds
	SmoothedData<double> m_gpuSmoothedData = SmoothedData(0.0); // seconds
};