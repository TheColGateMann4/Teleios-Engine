#include "RenderTechnique.h"
#include "Macros/ErrorMacros.h"

RenderTechnique::RenderTechnique(RenderJob::JobType type)
	:
	m_type(type)
{

}

RenderJob::JobType RenderTechnique::GetType() const
{
	return m_type;
}

void RenderTechnique::AddStep(RenderGraphicsStep&& step)
{
	m_steps.push_back(std::move(step));
}

RenderGraphicsStep& RenderTechnique::GetStep(unsigned int index)
{
	THROW_INTERNAL_ERROR_IF("Tried to access index out of bounds", index < 0 || index >= m_steps.size());

	return m_steps.at(index);
}

const RenderGraphicsStep& RenderTechnique::GetStep(unsigned int index) const
{
	THROW_INTERNAL_ERROR_IF("Tried to access index out of bounds", index < 0 || index >= m_steps.size());

	return m_steps.at(index);
}

std::vector<RenderGraphicsStep>& RenderTechnique::GetSteps()
{
	return m_steps;
}