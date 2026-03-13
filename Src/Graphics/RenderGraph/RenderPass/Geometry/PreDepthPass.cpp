#include "PreDepthPass.h"

RenderJob::JobType PreDepthPass::GetWantedJob() const
{
	return RenderJob::JobType::Depth;
}