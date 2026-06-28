#pragma once
#include "GeometryPass.h"

class EmissivePass : public GeometryPass
{
public:
	EmissivePass(Graphics& graphics);

	virtual RenderJob::JobType GetWantedJob() const override;
};