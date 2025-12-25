#include "RenderJob.h"

RenderJob::RenderJob(const std::string& categoryName, RenderStep* step)
	:
	m_categoryName(categoryName),
	m_step(step)
{

}

void RenderJob::Execute(Graphics& graphics, CommandList* commandList) const
{
	m_step->Execute(graphics, commandList);
}