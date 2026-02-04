#include "Bindable.h"
#include "Macros/ErrorMacros.h"

#include "Graphics/Core/RootSignature.h"

void CommandListBindable::BindToComputeCommandList(Graphics& graphics, CommandList* commandList)
{

}

void PipelineStateBindable::BindToComputePipelineState(Graphics& graphics, ComputePipelineState* pipelineState)
{

}

RootSignatureBindable::RootSignatureBindable(std::vector<TargetSlotAndShader> targets)
	:
	m_targets(targets)
{

}

void RootSignatureBindable::BindToComputeRootSignature(RootSignature* rootSignature)
{

}

void RootSignatureBindable::Initialize(Graphics& graphics)
{
	
}

std::vector<TargetSlotAndShader>& RootSignatureBindable::GetTargets()
{
	return m_targets;
}