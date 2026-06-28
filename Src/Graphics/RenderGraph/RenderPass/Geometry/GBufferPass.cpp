#include "GBufferPass.h"

#include "Includes/BindablesInclude.h"

GBufferPass::GBufferPass(Graphics& graphics)
{
	AddStaticBindable("lightBuffer");

	BlendStateOptions blendStateOptions = {}; // using the default options

	AddBindable(BlendState::GetResource(graphics, blendStateOptions));
	AddBindable(DepthStencilState::GetResource(graphics, DepthStencilState::DepthComparisonFunc::Equal));
	AddBindable(ViewPort::GetResource(graphics));
}

RenderJob::JobType GBufferPass::GetWantedJob() const
{
	return RenderJob::JobType::GBuffer;
}