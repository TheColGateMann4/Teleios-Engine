#include "RenderTechnique.h"
#include "Macros/ErrorMacros.h"

RenderTechnique::RenderTechnique(const std::string& name)
	:
	m_name(name)
{

}

const std::string& RenderTechnique::GetName() const
{
	return m_name;
}

void RenderTechnique::Initialize(Graphics& graphics, Pipeline& pipeline)
{
	for (auto& step : m_steps)
		step.Initialize(graphics, pipeline);
}

void RenderTechnique::Update(Graphics& graphics, Pipeline& pipeline)
{
	for (auto& step : m_steps)
		step.Update(graphics, pipeline);
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

std::vector<RenderGraphicsStep>& RenderTechnique::GetSteps()
{
	return m_steps;
}