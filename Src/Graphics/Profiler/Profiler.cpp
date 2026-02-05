#include "Profiler.h"

#include <imgui.h>

void Profiler::Initialize(Graphics& graphics)
{
	m_gpuProfiler.Initialize(graphics);
}

void Profiler::Draw()
{
	UpdateData();

	ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f), ImGuiCond_Always);
	ImGui::SetNextWindowBgAlpha(0.0f);
	ImGui::PushStyleColor(ImGuiCol_Border, 0);

	static constexpr ImGuiWindowFlags flags =
		ImGuiWindowFlags_NoDecoration |
		ImGuiWindowFlags_AlwaysAutoResize |
		ImGuiWindowFlags_NoInputs |
		ImGuiWindowFlags_NoNav |
		ImGuiWindowFlags_NoFocusOnAppearing |
		ImGuiWindowFlags_NoNav;

	if (ImGui::Begin("Profiler", nullptr, flags))
	{
		ImGui::Text("FPS: %.1f", m_fpsSmoothed.value);
		ImGui::Text("CPU: %.2f ms", m_cpuSmoothedData.value * 1000.0f);
		ImGui::Text("GPU: %.2f ms", m_gpuSmoothedData.value * 1000.0f);
	}

	ImGui::End();

	ImGui::PopStyleColor();
}

void Profiler::UpdateData()
{
	double gpuTime = m_gpuProfiler.GetData();	// seconds
	float cpuTime = m_cpuProfiler.GetData();	// seconds
	float frameTime = std::max<float>(cpuTime, gpuTime);
	float fps = frameTime > 0.0f ? 1.0f / frameTime : 0.0f;

	SmoothData(m_cpuSmoothedData, cpuTime);
	SmoothData(m_gpuSmoothedData, gpuTime);
	SmoothData(m_fpsSmoothed, fps);
}

void Profiler::SetBeginData(Graphics& graphics, CommandList* commandList, float deltaTime)
{
	m_cpuProfiler.SetBeginData(deltaTime);
	m_gpuProfiler.SetBeginData(graphics, commandList);
}

void Profiler::SetEndData(Graphics& graphics, CommandList* commandList, float deltaTime)
{
	m_cpuProfiler.EndBeginData(deltaTime);
	m_gpuProfiler.SetEndData(graphics, commandList);
}