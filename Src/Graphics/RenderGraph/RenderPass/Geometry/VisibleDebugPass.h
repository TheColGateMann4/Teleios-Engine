#pragma once
#include "GeometryPass.h"
class Graphics;

class VisibleDebugPass : public GeometryPass
{
public:
	VisibleDebugPass(Graphics& graphics);

public:
	virtual RenderJob::JobType GetWantedJob() const override;
};