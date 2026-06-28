#include "OccludedDebugPass.h"
#include "Graphics/Bindables/DepthStencilState.h"

OccludedDebugPass::OccludedDebugPass(Graphics& graphics)
{
	DepthStencilStateOptions depthStencilStateOptions = {};
	depthStencilStateOptions.SetDepthComparisonFunction(ComparisonFunction::Greater);

	AddBindable(DepthStencilState::GetResource(graphics, depthStencilStateOptions));
	m_rasterizerOptions.SetAliasedLine(true);
}

RenderJob::JobType OccludedDebugPass::GetWantedJob() const
{
	return RenderJob::JobType::OccludedDebug;
}