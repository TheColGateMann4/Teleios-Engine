#include "RenderPass.h"
#include "Graphics/Core/CommandList.h"
#include "Graphics/RenderGraph/RenderJob/RenderJob.h"

void RenderPass::Initialize(Graphics& graphics)
{

}

void RenderPass::InitializePassResources(Graphics& graphics, Pipeline& pipeline)
{

}

void RenderPass::SubmitJobs(RenderManager& renderManager)
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

void RenderPass::SetCorrectStates(Graphics& graphics, CommandList* commandList)
{
	for (auto& renderTarget : m_renderTargets)
		commandList->SetResourceState(graphics, renderTarget.get(), D3D12_RESOURCE_STATE_RENDER_TARGET);

	if (m_depthStencil)
		commandList->SetAllResourcesStates(graphics, m_depthStencil->GetResource(graphics), D3D12_RESOURCE_STATE_DEPTH_WRITE);
}

void RenderPass::Execute(Graphics& graphics, CommandList* commandList)
{
	BEGIN_COMMAND_LIST_EVENT(commandList, typeid(*this).name() + 6); // + 6 skips "class " from type info literal
	commandList->BeginRenderPass(graphics, this);

	ExecutePass(graphics, commandList);

	commandList->EndRenderPass(graphics);
	END_COMMAND_LIST_EVENT(commandList);
}