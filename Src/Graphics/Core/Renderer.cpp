#include "Renderer.h"

#include "Graphics/Core/Graphics.h"
#include "Scene/Scene.h"
#include "Scene/SceneObject.h"

void Renderer::Initialize(Graphics& graphics)
{
	m_pipeline.Initialize(graphics);
	m_imguiLayer.Initialize(graphics);
	m_renderGraph.Initialize(graphics);
}

void Renderer::GatherJobBindables()
{
	m_renderGraph.GatherJobBindables();
}

void Renderer::InitializeJobs(Graphics& graphics)
{
	m_renderGraph.InitializeJobs(graphics, m_pipeline);
}

void Renderer::InitializePasses(Graphics& graphics, Scene& scene)
{
	m_renderGraph.InitializePasses(graphics, m_pipeline, scene);
}

void Renderer::FinishInitialization(Graphics& graphics)
{
	m_pipeline.FinishInitialization(graphics);
}

void Renderer::SubmitPassesJobs()
{
	m_renderGraph.SubmitPassesJobs();
}

void Renderer::AssignJobsToPasses()
{
	m_renderGraph.AssignJobsToPasses();
}

void Renderer::Draw(Graphics& graphics)
{
	{
		// moved to graphics::BeginFrame(), 
		// we need to push copy events during runtime, we need to have command list open then
		// in future we will set up system that will synchronise command lists to support more dynamic and clean workflow
		//m_pipeline.BeginRender(graphics);

		//scene.InitializeGraphicResources(graphics);

		m_pipeline.ExecuteCopyCalls(graphics);

		m_renderGraph.Execute(graphics, m_pipeline.GetGraphicCommandList());

		m_pipeline.FinishRender(graphics);
	}

	// executing command lists
	m_pipeline.Execute(graphics);
}

void Renderer::DrawImguiWindow(Graphics& graphics)
{
	//m_renderGraph.Update(graphics, m_pipeline);
}

void Renderer::SubmitJob(std::shared_ptr<RenderJob> job)
{
	m_renderGraph.GetRenderManager().AddJob(std::move(job));
}

Pipeline& Renderer::GetPipeline()
{
	return m_pipeline;
}

ImguiLayer& Renderer::GetImguiLayer()
{
	return m_imguiLayer;
}