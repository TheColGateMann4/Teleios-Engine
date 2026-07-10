#include "RenderGraph.h"
#include "Graphics/Core/Graphics.h"

#include "RenderPass/Geometry/PreDepthPass.h"
#include "RenderPass/Geometry/GBufferPass.h"
#include "RenderPass/Geometry/EmissivePass.h"
#include "RenderPass/Geometry/ShadowPass.h"
#include "RenderPass/Fullscreen/FullscreenPlaceholderPass.h"
#include "RenderPass/Fullscreen/LightningPass.h"
#include "RenderPass/Fullscreen/SkyBoxPass.h"
#include "RenderPass/GuiPass.h"
#include "RenderPass/Geometry/OccludedDebugPass.h"
#include "RenderPass/Geometry/VisibleDebugPass.h"

void RenderGraph::Initialize(Graphics& graphics)
{
	{
		std::shared_ptr<PreDepthPass> preDepthPass = std::make_shared<PreDepthPass>(graphics);
		preDepthPass->SetDepthStencilView(graphics.GetDepthStencil(), ResourceDataOperation::clear);

		AddRenderPass(preDepthPass);
	}

	std::shared_ptr<DepthStencilViewCubeMultiResource> lightDepthData = std::make_shared<DepthStencilViewCubeMultiResource>(graphics);
	{
		std::shared_ptr<ShadowPass> shadowPass = std::make_shared<ShadowPass>(graphics);
		shadowPass->SetDepthStencilView(lightDepthData, ResourceDataOperation::clear);
	
		AddRenderPass(shadowPass);
	}

	DXGI_FORMAT backBufferFormat = graphics.GetBackBuffer()->GetFormat();
	std::shared_ptr<BackBufferRenderTarget> rt0 = std::make_shared<BackBufferRenderTarget>(graphics, backBufferFormat);
	std::shared_ptr<BackBufferRenderTarget> rt1 = std::make_shared<BackBufferRenderTarget>(graphics, backBufferFormat);
	std::shared_ptr<BackBufferRenderTarget> rt2 = std::make_shared<BackBufferRenderTarget>(graphics, backBufferFormat);
	{
		std::shared_ptr<GBufferPass> geometryPass = std::make_shared<GBufferPass>(graphics);
		geometryPass->AddRenderTarget(rt0, ResourceDataOperation::clear);
		geometryPass->AddRenderTarget(rt1, ResourceDataOperation::clear);
		geometryPass->AddRenderTarget(rt2, ResourceDataOperation::clear);
		geometryPass->SetDepthStencilView(graphics.GetDepthStencil());
	
		AddRenderPass(geometryPass);
	}

	std::shared_ptr<ShaderResourceViewMultiResource> rt0srv = std::make_shared<ShaderResourceViewMultiResource>(graphics, rt0.get(), 0);
	std::shared_ptr<ShaderResourceViewMultiResource> rt1srv = std::make_shared<ShaderResourceViewMultiResource>(graphics, rt1.get(), 1);
	std::shared_ptr<ShaderResourceViewMultiResource> rt2srv = std::make_shared<ShaderResourceViewMultiResource>(graphics, rt2.get(), 2);
	std::shared_ptr<ShaderResourceViewMultiResource> depthsrv = std::make_shared<ShaderResourceViewMultiResource>(graphics, graphics.GetDepthStencil().get(), 3);
	std::shared_ptr<ShaderResourceViewMultiResource> shadowMap = std::make_shared<ShaderResourceViewMultiResource>(graphics, lightDepthData.get(), 4);
	{
		std::shared_ptr<LightningPass> lightningPass = std::make_shared<LightningPass>(graphics);
		lightningPass->AddRenderTarget(graphics.GetBackBuffer());
		lightningPass->AddBindable(rt0srv);
		lightningPass->AddBindable(rt1srv);
		lightningPass->AddBindable(rt2srv);
		lightningPass->AddBindable(depthsrv);
		lightningPass->AddBindable(shadowMap);

		AddRenderPass(lightningPass);
	}

	{
		std::shared_ptr<EmissivePass> emissivePass = std::make_shared<EmissivePass>(graphics);
		emissivePass->AddRenderTarget(graphics.GetBackBuffer());
		emissivePass->SetDepthStencilView(graphics.GetDepthStencil());

		AddRenderPass(emissivePass);
	}

	{
		std::shared_ptr<SkyboxPass> skyBoxPass = std::make_shared<SkyboxPass>(graphics);
		skyBoxPass->AddRenderTarget(graphics.GetBackBuffer());
		skyBoxPass->SetDepthStencilView(graphics.GetDepthStencil());
		AddRenderPass(skyBoxPass);
	}

	{
		std::shared_ptr<VisibleDebugPass> visibleDebugPass = std::make_shared<VisibleDebugPass>(graphics);
		visibleDebugPass->AddRenderTarget(graphics.GetBackBuffer());
		visibleDebugPass->SetDepthStencilView(graphics.GetDepthStencil());

		AddRenderPass(visibleDebugPass);
	}

	{
		std::shared_ptr<OccludedDebugPass> occludedDebugPass = std::make_shared<OccludedDebugPass>(graphics);
		occludedDebugPass->AddRenderTarget(graphics.GetBackBuffer());
		occludedDebugPass->SetDepthStencilView(graphics.GetDepthStencil());

		AddRenderPass(occludedDebugPass);
	}

	{
		std::shared_ptr<FullscreenPlaceholderPass> fullscreenPass = std::make_shared<FullscreenPlaceholderPass>(graphics);
		fullscreenPass->AddRenderTarget(graphics.GetSwapChainBuffer(), ResourceDataOperation::discard);
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
	for (auto& geometryPass : m_geometryPasses)
		geometryPass->GatherJobBindables();
}

void RenderGraph::InitializeJobs(Graphics& graphics, Pipeline& pipeline)
{
	for (auto& geometryPass : m_geometryPasses)
		geometryPass->InitializeJobs(graphics, pipeline);
}

void RenderGraph::InitializePasses(Graphics& graphics, Pipeline& pipeline, Scene& scene)
{
	for (auto& renderPass : m_renderPasses)
	{
		renderPass->Initialize(graphics, scene);
		renderPass->InitializePassResources(graphics, pipeline, scene);
	}
}

void RenderGraph::UpdatePasses(Graphics& graphics, Pipeline& pipeline, Scene& scene)
{
	for (auto& renderPass : m_renderPasses)
		renderPass->Update(graphics, pipeline, scene);
}

void RenderGraph::SubmitPassesJobs()
{
	for (auto& renderPass : m_renderPasses)
		renderPass->SubmitJobs(GetRenderManager());
}

void RenderGraph::AssignNewJobsToPasses()
{
	m_renderManager.AssignNewJobsToPasses(m_geometryPasses);
}

void RenderGraph::Execute(Graphics& graphics, CommandList* commandList, Scene& scene)
{
	for (auto& renderPass : m_renderPasses)
	{
		renderPass->SetCorrectStates(graphics, commandList);
		renderPass->Execute(graphics, commandList, scene);
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