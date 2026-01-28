#pragma once
#include "GeometryPass.h"

class GBufferPass : public GeometryPass
{
	virtual RenderJob::JobType GetWantedJob() const override;
};