#pragma once
#include "GraphicsStepRenderJob.h"

class RenderPass;
class Scene;

// It is normal Graphics step, but doesn't use occlusion checks
class DebugGraphicsStepRenderJob : public GraphicsStepRenderJob
{
public:
	DebugGraphicsStepRenderJob(GraphicsRenderData renderData, GeometryPass* pass);

public:
	virtual bool IsValid(RenderPass* pass, Scene& scene) const override;
};