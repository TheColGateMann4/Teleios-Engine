#include "Bindable.h"
#include "Macros/ErrorMacros.h"


RootSignatureBindable::RootSignatureBindable(std::vector<TargetSlotAndShader> targets)
	:
	m_targets(targets)
{

}

std::vector<TargetSlotAndShader>& RootSignatureBindable::GetTargets()
{
	return m_targets;
}