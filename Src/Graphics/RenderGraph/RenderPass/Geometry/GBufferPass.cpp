#include "GBufferPass.h"

#include "Includes/BindablesInclude.h"

GBufferPass::GBufferPass(Graphics& graphics)
{
	AddStaticBindable("lightBuffer");

	AddBindable(BlendState::GetResource(graphics, false));
	AddBindable(DepthStencilState::GetResource(graphics, DepthStencilState::DepthComparisonFunc::Equal));
	AddBindable(ViewPort::GetResource(graphics));
}

RenderJob::JobType GBufferPass::GetWantedJob() const
{
	return RenderJob::JobType::GBuffer;
}