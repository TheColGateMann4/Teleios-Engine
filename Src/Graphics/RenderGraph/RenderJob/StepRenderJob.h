#pragma once
#include "RenderJob.h"

class RenderStep;

class StepRenderJob : public RenderJob
{
public:
	StepRenderJob(RenderJob::JobType m_type, RenderStep* step);

public:
	virtual void Execute(Graphics& graphics, CommandList* commandList) const override;

private:
	RenderStep* m_step;
};