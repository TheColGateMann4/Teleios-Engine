#include "DebugGraphicsStepRenderJob.h"
#include "Graphics/RenderGraph/Steps/RenderGraphicsStep.h"

DebugGraphicsStepRenderJob::DebugGraphicsStepRenderJob(GraphicsRenderData renderData, GeometryPass* pass)
	:
	GraphicsStepRenderJob(renderData, pass)
{

}

bool DebugGraphicsStepRenderJob::IsValid(RenderPass* pass, Scene& scene) const
{
	return m_step->IsEnabled();
}