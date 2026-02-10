#include "GBufferPass.h"

#include "Includes/BindablesInclude.h"

GBufferPass::GBufferPass(Graphics& graphics)
{
	AddBindable(BlendState::GetBindableResource(graphics, false));
	AddBindable(DepthStencilState::GetBindableResource(graphics, DepthStencilState::DepthComparisonFunc::Equal));
}

RenderJob::JobType GBufferPass::GetWantedJob() const
{
	return RenderJob::JobType::GBuffer;
}