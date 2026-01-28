#include "GBufferPass.h"

RenderJob::JobType GBufferPass::GetWantedJob() const
{
	return RenderJob::JobType::None;
}