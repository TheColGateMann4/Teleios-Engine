#include "GeometryPass.h"

RenderJob::JobType GeometryPass::GetWantedJob() const
{
	return RenderJob::JobType::GeometryPass;
}