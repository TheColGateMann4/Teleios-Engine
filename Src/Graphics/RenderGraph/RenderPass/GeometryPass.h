#pragma once
#include "RenderPass.h"

class GeometryPass : public RenderPass
{
public:
	virtual RenderJob::JobType GetWantedJob() const override;
};