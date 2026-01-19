#include "RenderGraph.h"
#include "Graphics/Core/Graphics.h"

#include "RenderPass/PreDepthPass.h"
#include "RenderPass/GeometryPass.h"
#include "RenderPass/Fullscreen/FullscreenRenderPass.h"
#include "RenderPass/GuiPass.h"

void RenderGraph::Initialize(Graphics& graphics)
{
	{
		std::shared_ptr<PreDepthPass> preDepthPass = std::make_shared<PreDepthPass>();
		preDepthPass->SetDepthStencilView(graphics.GetDepthStencil());

		AddRenderPass(preDepthPass);
	}

	{
		std::shared_ptr<GeometryPass> geometryPass = std::make_shared<GeometryPass>();
		geometryPass->AddRenderTarget(graphics.GetBackBuffer());
		geometryPass->SetDepthStencilView(graphics.GetDepthStencil());

		AddRenderPass(geometryPass);
	}

	{
		std::shared_ptr<FullscreenRenderPass> fullscreenPass = std::make_shared<FullscreenRenderPass>(graphics, GetRenderManager());
		fullscreenPass->AddRenderTarget(graphics.GetSwapChainBuffer());
		AddRenderPass(fullscreenPass);
	}

	{
		std::shared_ptr<GuiPass> guiPass = std::make_shared<GuiPass>(graphics);
		guiPass->AddRenderTarget(graphics.GetSwapChainBuffer());
		AddRenderPass(guiPass);
	}
}

void RenderGraph::GatherJobBindables()
{
	m_renderManager.GatherJobBindables();
}

void RenderGraph::InitializeJobs(Graphics& graphics, Pipeline& pipeline)
{
	m_renderManager.InitializeJobs(graphics, pipeline);
}

void RenderGraph::InitializePasses(Graphics& graphics, Pipeline& pipeline)
{
	for (auto& renderPass : m_renderPasses)
	{
		renderPass->Initialize(graphics);
		renderPass->InitializePassResources(graphics, pipeline);
	}
}

void RenderGraph::SubmitPassesJobs()
{
	for (auto& renderPass : m_renderPasses)
		renderPass->SubmitJobs(GetRenderManager());
}

void RenderGraph::AssignJobsToPasses()
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