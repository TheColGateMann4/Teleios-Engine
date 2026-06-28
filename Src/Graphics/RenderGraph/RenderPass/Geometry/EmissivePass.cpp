#include "EmissivePass.h"

#include "Graphics/Bindables/DepthStencilState.h"
#include "Graphics/Bindables/BlendState.h"

EmissivePass::EmissivePass(Graphics& graphics)
{
	AddBindable(BlendState::GetResource(graphics, BlendStateOptions{}));
	AddBindable(DepthStencilState::GetResource(graphics, DepthStencilStateOptions{}));
}

RenderJob::JobType EmissivePass::GetWantedJob() const
{
	return RenderJob::JobType::Emissive;
}