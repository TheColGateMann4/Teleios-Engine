#include "Bindable.h"
#include "Macros/ErrorMacros.h"

#include "Graphics/Core/RootSignature.h"

void CommandListBindable::BindToComputeCommandList(Graphics& graphics, CommandList* commandList)
{

}

void PipelineStateBindable::AddComputePipelineStateParam(Graphics& graphics, ComputePipelineStateParams* params)
{

}

RootSignatureBindable::RootSignatureBindable(std::vector<TargetSlotAndShader> targets)
	:
	m_targets(targets)
{

}

void RootSignatureBindable::AddComputeRootSignatureParam(RootSignatureParams* rootSignatureParams)
{

}

std::vector<TargetSlotAndShader>& RootSignatureBindable::GetTargets()
{
	return m_targets;
}