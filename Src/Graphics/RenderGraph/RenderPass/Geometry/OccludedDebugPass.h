#pragma once
#include "GeometryPass.h"

class OccludedDebugPass : public GeometryPass
{
public:
	OccludedDebugPass(Graphics& graphics);

public:
	virtual RenderJob::JobType GetWantedJob() const override;
};