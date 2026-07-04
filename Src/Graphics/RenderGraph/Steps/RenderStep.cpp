#include "RenderStep.h"

RenderStep::RenderStep()
	:
	m_name("Unnamed")
{

}

RenderStep::RenderStep(const std::string& name)
	:
	m_name(name)
{

}

const std::string& RenderStep::GetName() const
{
	return m_name;
}

bool RenderStep::IsEnabled() const
{
	return m_enabled;
}

void RenderStep::SetEnabled(bool enabled)
{
	m_enabled = enabled;
}

void RenderStep::Initialize(Graphics& graphics, Pipeline& pipeline)
{

}

void RenderStep::Update()
{

}