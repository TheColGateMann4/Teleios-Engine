#include "RenderGraph.h"
#include "Graphics/Core/Graphics.h"

#include "RenderPass/Geometry/PreDepthPass.h"
#include "RenderPass/Geometry/GBufferPass.h"
#include "RenderPass/Fullscreen/FullscreenRenderPass.h"
#include "RenderPass/Fullscreen/LightningPass.h"
#include "RenderPass/GuiPass.h"

void RenderGraph::Initialize(Graphics& graphics)
{
	{
		std::shared_ptr<PreDepthPass> preDepthPass = std::make_shared<PreDepthPass>();
		preDepthPass->SetDepthStencilView(graphics.GetDepthStencil());

		AddRenderPass(preDepthPass);
	}


	DXGI_FORMAT backBufferFormat = graphics.GetBackBuffer()->GetFormat();
	std::shared_ptr<BackBufferRenderTarget> rt0 = std::make_shared<BackBufferRenderTarget>(graphics, backBufferFormat);
	std::shared_ptr<BackBufferRenderTarget> rt1 = std::make_shared<BackBufferRenderTarget>(graphics, backBufferFormat);
	std::shared_ptr<BackBufferRenderTarget> rt2 = std::make_shared<BackBufferRenderTarget>(graphics, backBufferFormat);

	{
		std::shared_ptr<GBufferPass> geometryPass = std::make_shared<GBufferPass>();
		geometryPass->AddRenderTarget(rt0);
		geometryPass->AddRenderTarget(rt1);
		geometryPass->AddRenderTarget(rt2);
		geometryPass->SetDepthStencilView(graphics.GetDepthStencil());
	
		AddRenderPass(geometryPass);
	}

	std::shared_ptr<ShaderResourceViewMultiResource> rt0srv = std::make_shared<ShaderResourceViewMultiResource>(graphics, rt0.get(), 0);
	std::shared_ptr<ShaderResourceViewMultiResource> rt1srv = std::make_shared<ShaderResourceViewMultiResource>(graphics, rt1.get(), 1);
	std::shared_ptr<ShaderResourceViewMultiResource> rt2srv = std::make_shared<ShaderResourceViewMultiResource>(graphics, rt2.get(), 2);
	std::shared_ptr<ShaderResourceViewMultiResource> depthsrv = std::make_shared<ShaderResourceViewMultiResource>(graphics, graphics.GetDepthStencil().get(), 3);

	{
		std::shared_ptr<LightningPass> geometryPass = std::make_shared<LightningPass>(graphics, GetRenderManager());
		geometryPass->AddRenderTarget(graphics.GetBackBuffer());
		geometryPass->AddBindable(rt0srv);
		geometryPass->AddBindable(rt1srv);
		geometryPass->AddBindable(rt2srv);
		geometryPass->AddBindable(depthsrv);

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
	m_renderManager.BindJobsToPasses(m_geometryPasses);
}

void RenderGraph::Execute(Graphics& graphics, CommandList* commandList)
{
	for (auto& renderPass : m_renderPasses)
	{
		renderPass->SetCorrectStates(graphics, commandList);
		renderPass->Execute(graphics, commandList);
	}
}

RenderManager& RenderGraph::GetRenderManager()
{
	return m_renderManager;
}

void RenderGraph::AddRenderPass(std::shared_ptr<RenderPass> renderPass)
{
	if (GeometryPass* geometryPass = dynamic_cast<GeometryPass*>(renderPass.get()))
		m_geometryPasses.push_back(geometryPass);

	m_renderPasses.push_back(renderPass);
}