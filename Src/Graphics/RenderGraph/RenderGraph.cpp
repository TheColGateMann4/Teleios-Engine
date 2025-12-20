#include "RenderGraph.h"

#include "Graphics/Core/Graphics.h"
#include "Scene/Scene.h"
#include "Scene/SceneObject.h"

void RenderGraph::Initialize(Graphics& graphics)
{
	m_pipeline.Initialize(graphics);
	m_imguiLayer.Initialize(graphics);
	m_postProcessing.Initialize(graphics);
}

void RenderGraph::FinishInitialization(Graphics& graphics)
{
	m_postProcessing.InitializeResources(graphics, m_pipeline);

	m_pipeline.FinishInitialization(graphics);
}

void RenderGraph::Draw(Graphics& graphics, Scene& scene)
{
	{
		m_pipeline.BeginRender(graphics);

		scene.InitializeGraphicResources(graphics);

		m_pipeline.ExecuteCopyCalls(graphics);

		// drawing scene objects
		{
			auto& sceneObjects = scene.GetObjects();

			for (auto& sceneObject : sceneObjects)
				sceneObject->InternalDraw(graphics, m_pipeline);
		}

		// drawing imgui layer
		{
			m_imguiLayer.Render();

			if (m_imguiLayer.IsVisible())
				m_imguiLayer.Draw(graphics, m_pipeline);
		}


		// apply post processing
		m_postProcessing.ApplyEffect(graphics, m_pipeline);

		m_pipeline.FinishRender(graphics);
	}

	// executing command lists
	m_pipeline.Execute(graphics);
}

void RenderGraph::DrawImguiWindow(Graphics& graphics)
{
	m_postProcessing.Update(graphics, m_pipeline);
}

Pipeline& RenderGraph::GetPipeline()
{
	return m_pipeline;
}

ImguiLayer& RenderGraph::GetImguiLayer()
{
	return m_imguiLayer;
}