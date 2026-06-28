#include "OccludedDebugPass.h"
#include "Graphics/Bindables/DepthStencilState.h"

OccludedDebugPass::OccludedDebugPass(Graphics& graphics)
{
	AddBindable(DepthStencilState::GetResource(graphics, DepthStencilState::DepthComparisonFunc::Greater));
	m_rasterizerOptions.SetAliasedLine(true);
}

RenderJob::JobType OccludedDebugPass::GetWantedJob() const
{
	return RenderJob::JobType::OccludedDebug;
}