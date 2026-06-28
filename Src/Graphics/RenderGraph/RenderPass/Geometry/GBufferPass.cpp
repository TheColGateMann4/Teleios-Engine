#include "GBufferPass.h"

#include "Includes/BindablesInclude.h"

GBufferPass::GBufferPass(Graphics& graphics)
{
	AddStaticBindable("lightBuffer");

	AddBindable(BlendState::GetResource(graphics, BlendStateOptions{}));

	DepthStencilStateOptions depthStencilStateOptions = {};
	depthStencilStateOptions.SetDepthComparisonFunction(ComparisonFunction::Equal);

	AddBindable(DepthStencilState::GetResource(graphics, depthStencilStateOptions));
	AddBindable(ViewPort::GetResource(graphics));
}

RenderJob::JobType GBufferPass::GetWantedJob() const
{
	return RenderJob::JobType::GBuffer;
}