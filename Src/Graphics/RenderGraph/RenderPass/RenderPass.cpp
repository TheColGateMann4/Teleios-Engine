#include "RenderPass.h"
#include "Graphics/Core/CommandList.h"
#include "Graphics/RenderGraph/RenderJob/RenderJob.h"

void RenderPass::Initialize(Graphics& graphics, Pipeline& pipeline, RenderManager& renderManager)
{

}

void RenderPass::AddRenderTarget(std::shared_ptr<RenderTarget> renderTarget)
{
	m_renderTargets.push_back(renderTarget);
}

void RenderPass::SetDepthStencilView(std::shared_ptr<DepthStencilViewBase> depthStencil)
{
	m_depthStencil = depthStencil;
}

const std::vector<std::shared_ptr<RenderTarget>>& RenderPass::GetRenderTargets() const
{
	return m_renderTargets;
}

std::shared_ptr<DepthStencilViewBase> RenderPass::GetDepthStencilView() const
{
	return m_depthStencil;
}

void RenderPass::SortJobs()
{
	// TODO: sort jobs by their PSO
}

void RenderPass::Execute(Graphics& graphics, CommandList* commandList)
{
	commandList->BeginRenderPass(graphics, this);

	PreDraw(graphics, commandList);

	for (auto pJob : m_pJobs)
		pJob->Execute(graphics, commandList);

	PostDraw(graphics, commandList);

	commandList->EndRenderPass(graphics);
}

void RenderPass::PreDraw(Graphics& graphics, CommandList* commandList)
{

}

void RenderPass::PostDraw(Graphics& graphics, CommandList* commandList)
{

}

RenderJob::JobType RenderPass::GetWantedJob() const
{
	return RenderJob::JobType::None;
}

void RenderPass::AssignJob(std::shared_ptr<RenderJob> pJob)
{
	m_pJobs.push_back(pJob);
}