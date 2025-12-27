#include "RenderJob.h"

RenderJob::RenderJob(JobType type, RenderStep* step)
	:
	m_type(type),
	m_step(step)
{

}

void RenderJob::Execute(Graphics& graphics, CommandList* commandList) const
{
	m_step->Execute(graphics, commandList);
}

RenderJob::JobType RenderJob::GetType() const
{
	return m_type;
}