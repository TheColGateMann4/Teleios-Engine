#pragma once
#include "Graphics/Core/Pipeline.h"
#include "Graphics/Imgui/ImguiLayer.h"
#include "Graphics/Core/PostProcessing.h"
#include "Graphics/RenderGraph/RenderJob.h"

class Graphics;
class Scene;

class RenderGraph
{
public:
	void Initialize(Graphics& graphics);
	void FinishInitialization(Graphics& graphics);

	void Draw(Graphics& graphics);

	void DrawImguiWindow(Graphics& graphics);

	void SubmitJob(RenderJob&& job);

public:
	Pipeline& GetPipeline();
	ImguiLayer& GetImguiLayer();

private:
	Pipeline m_pipeline;
	ImguiLayer m_imguiLayer;
	PostProcessing m_postProcessing;

	std::vector<RenderJob> m_renderJobs;
};