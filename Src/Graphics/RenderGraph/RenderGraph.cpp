#include "RenderGraph.h"
#include "Graphics/Core/Graphics.h"

#include "RenderPass/GeometryPass.h"
#include "RenderPass/Fullscreen/FullscreenRenderPass.h"

void RenderGraph::Initialize(Graphics& graphics)
{
	{
		std::shared_ptr<GeometryPass> geometryPass = std::make_shared<GeometryPass>();
		geometryPass->AddRenderTarget(graphics.GetBackBuffer());
		geometryPass->SetDepthStencilView(graphics.GetDepthStencil());

		AddRenderPass(geometryPass);
	}

	{
		std::shared_ptr<FullscreenRenderPass> fullscreenRenderPass = std::make_shared<FullscreenRenderPass>(graphics, GetRenderManager());
		fullscreenRenderPass->AddRenderTarget(graphics.GetSwapChainBuffer());
		AddRenderPass(fullscreenRenderPass);
	}
}

void RenderGraph::InitializePassResources(Graphics& graphics, Pipeline& pipeline)
{
	for (auto& renderPass : m_renderPasses)
		renderPass->Initialize(graphics, pipeline, GetRenderManager());
}

void RenderGraph::RebindJobs()
{
	m_renderManager.BindJobsToPasses(m_renderPasses);
}

void RenderGraph::Execute(Graphics& graphics, CommandList* commandList)
{
	for (auto& renderPass : m_renderPasses)
		renderPass->Execute(graphics, commandList);
}

RenderManager& RenderGraph::GetRenderManager()
{
	return m_renderManager;
}

void RenderGraph::AddRenderPass(std::shared_ptr<RenderPass> renderPass)
{
	m_renderPasses.push_back(renderPass);
}