#include "EmissivePass.h"

RenderJob::JobType EmissivePass::GetWantedJob() const
{
	return RenderJob::JobType::Emissive;
}