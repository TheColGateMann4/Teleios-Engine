#pragma once
#include "GeometryPass.h"

class GBufferPass : public GeometryPass
{
public:
	virtual RenderJob::JobType GetWantedJob() const override;
};