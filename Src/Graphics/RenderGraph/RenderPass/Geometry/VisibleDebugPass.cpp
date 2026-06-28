#include "VisibleDebugPass.h"
#include "Graphics/Bindables/DepthStencilState.h"

VisibleDebugPass::VisibleDebugPass(Graphics& graphics)
{
	AddBindable(DepthStencilState::GetResource(graphics, DepthStencilState::DepthComparisonFunc::Less));
	m_rasterizerOptions.SetAliasedLine(true);
}

RenderJob::JobType VisibleDebugPass::GetWantedJob() const
{
	return RenderJob::JobType::VisibleDebug;
}