#include "PreDepthPass.h"

PreDepthPass::PreDepthPass(Graphics& graphics)
{
	AddStaticBindable("cameraBuffer");
}


RenderJob::JobType PreDepthPass::GetWantedJob() const
{
	return RenderJob::JobType::Depth;
}