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
	void FinishInitialization(Graphics& graphics);

	void Draw(Graphics& graphics);

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