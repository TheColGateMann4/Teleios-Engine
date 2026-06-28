#include "VisibleDebugPass.h"
#include "Graphics/Bindables/DepthStencilState.h"

VisibleDebugPass::VisibleDebugPass(Graphics& graphics)
{
	AddBindable(DepthStencilState::GetResource(graphics, DepthStencilStateOptions{}));
	m_rasterizerOptions.SetAliasedLine(true);
}

RenderJob::JobType VisibleDebugPass::GetWantedJob() const
{
	return RenderJob::JobType::VisibleDebug;
}