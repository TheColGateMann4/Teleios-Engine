#pragma once
#include "GeometryPass.h"

class PreDepthPass : public GeometryPass
{
public:
	virtual RenderJob::JobType GetWantedJob() const override;
};