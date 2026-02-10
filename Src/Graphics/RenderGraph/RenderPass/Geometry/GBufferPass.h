#pragma once
#include "GeometryPass.h"

class GBufferPass : public GeometryPass
{
public:
	GBufferPass(Graphics& graphics);

	virtual RenderJob::JobType GetWantedJob() const override;
};