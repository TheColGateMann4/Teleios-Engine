#pragma once
#include "GeometryPass.h"

class EmissivePass : public GeometryPass
{
public:
	virtual RenderJob::JobType GetWantedJob() const override;
};