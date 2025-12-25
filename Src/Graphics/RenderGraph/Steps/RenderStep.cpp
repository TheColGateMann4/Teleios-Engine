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