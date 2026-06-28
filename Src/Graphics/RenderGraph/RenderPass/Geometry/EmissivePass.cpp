#include "EmissivePass.h"

#include "Graphics/Bindables/DepthStencilState.h"
#include "Graphics/Bindables/BlendState.h"

EmissivePass::EmissivePass(Graphics& graphics)
{
	BlendStateOptions blendStateOptions = {}; // using the default options

	AddBindable(BlendState::GetResource(graphics, blendStateOptions));

	AddBindable(DepthStencilState::GetResource(graphics, DepthStencilState::DepthComparisonFunc::Less));
}

RenderJob::JobType EmissivePass::GetWantedJob() const
{
	return RenderJob::JobType::Emissive;
}