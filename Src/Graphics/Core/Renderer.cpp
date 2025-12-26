#include "Renderer.h"

#include "Graphics/Core/Graphics.h"
#include "Scene/Scene.h"
#include "Scene/SceneObject.h"

void Renderer::Initialize(Graphics& graphics)
{
	m_pipeline.Initialize(graphics);
	m_imguiLayer.Initialize(graphics);
	m_postProcessing.Initialize(graphics);
}

void Renderer::FinishInitialization(Graphics& graphics)
{
	m_postProcessing.InitializeResources(graphics, m_pipeline);

	m_pipeline.FinishInitialization(graphics);
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

		// executing graphics render jobs
		{
			for (auto& job : m_renderJobs)
				job.Execute(graphics, m_pipeline.GetGraphicCommandList());
		}

		// apply post processing
		m_postProcessing.ApplyEffect(graphics, m_pipeline);

		// drawing imgui layer
		{
			m_imguiLayer.Render();

			if (m_imguiLayer.IsVisible())
				m_imguiLayer.Draw(graphics, m_pipeline);
		}

		m_pipeline.FinishRender(graphics);
	}

	// executing command lists
	m_pipeline.Execute(graphics);
}

void Renderer::DrawImguiWindow(Graphics& graphics)
{
	m_postProcessing.Update(graphics, m_pipeline);
}

void Renderer::SubmitJob(RenderJob&& job)
{
	m_renderJobs.push_back(job);
}

Pipeline& Renderer::GetPipeline()
{
	return m_pipeline;
}

ImguiLayer& Renderer::GetImguiLayer()
{
	return m_imguiLayer;
}