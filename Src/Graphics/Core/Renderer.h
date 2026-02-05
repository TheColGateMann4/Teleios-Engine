#pragma once
#include "Graphics/Core/Pipeline.h"
#include "Graphics/Imgui/ImguiLayer.h"
#include "Graphics/RenderGraph/RenderGraph.h"

class Graphics;
class Scene;

class Renderer
{
public:
	void Initialize(Graphics& graphics);
	void GatherJobBindables();
	void InitializeJobs(Graphics& graphics);
	void InitializePasses(Graphics& graphics, Scene& scene);
	void UpdatePasses(Graphics& graphics, Scene& scene);
	void FinishInitialization(Graphics& graphics);
	void SubmitPassesJobs();
	void AssignJobsToPasses();

	void Draw(Graphics& graphics, float deltaTime);

	void DrawImguiWindow(Graphics& graphics);

	void SubmitJob(std::shared_ptr<RenderJob> job);

public:
	Pipeline& GetPipeline();
	ImguiLayer& GetImguiLayer();

private:
	Pipeline m_pipeline;
	ImguiLayer m_imguiLayer;
	RenderGraph m_renderGraph;
};