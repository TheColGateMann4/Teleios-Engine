#include "PreDepthPass.h"

#include "Graphics/Bindables/DepthStencilState.h"

PreDepthPass::PreDepthPass(Graphics& graphics)
{
	AddBindable(DepthStencilState::GetResource(graphics, DepthStencilState::DepthComparisonFunc::Less));
}

RenderJob::JobType PreDepthPass::GetWantedJob() const
{
	return RenderJob::JobType::Depth;
}