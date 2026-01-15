#pragma once
#include "RenderPass.h"

class PreDepthPass : public RenderPass
{
public:
	virtual RenderJob::JobType GetWantedJob() const override;
};