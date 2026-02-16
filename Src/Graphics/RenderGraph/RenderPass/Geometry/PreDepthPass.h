#pragma once
#include "GeometryPass.h"

class PreDepthPass : public GeometryPass
{
public:
	PreDepthPass(Graphics& graphics);

	virtual RenderJob::JobType GetWantedJob() const override;
};