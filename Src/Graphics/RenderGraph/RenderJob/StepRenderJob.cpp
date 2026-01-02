#include "StepRenderJob.h"
#include "Graphics/RenderGraph/Steps/RenderStep.h"

StepRenderJob::StepRenderJob(JobType type, RenderStep* step)
	:
	RenderJob(type),
	m_step(step)
{

}

void StepRenderJob::Execute(Graphics& graphics, CommandList* commandList) const
{
	m_step->Execute(graphics, commandList);
}