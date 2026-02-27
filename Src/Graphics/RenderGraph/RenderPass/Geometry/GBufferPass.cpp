#include "GBufferPass.h"

#include "Includes/BindablesInclude.h"

GBufferPass::GBufferPass(Graphics& graphics)
{
	AddStaticBindable("lightBuffer");
	AddStaticBindable("cameraBuffer");

	AddBindable(BlendState::GetResource(graphics, false));
	AddBindable(DepthStencilState::GetResource(graphics, DepthStencilState::DepthComparisonFunc::Equal));
}

RenderJob::JobType GBufferPass::GetWantedJob() const
{
	return RenderJob::JobType::GBuffer;
}