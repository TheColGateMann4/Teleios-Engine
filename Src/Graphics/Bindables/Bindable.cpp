#include "Bindable.h"
#include "Macros/ErrorMacros.h"

#include "Graphics/Core/RootSignature.h"

void UpdatableBindable::SetUpdated()
{
	m_revision++;
}

unsigned long long UpdatableBindable::GetRevision() const
{
	return m_revision;
}

void CommandListBindable::BindToComputeCommandList(Graphics& graphics, CommandList* commandList)
{

}

void PipelineStateBindable::AddComputePipelineStateParam(Graphics& graphics, ComputePipelineStateParams* params)
{

}

RootSignatureBindable::RootSignatureBindable(ResourceTargets targets)
	:
	m_targets(targets)
{

}

void RootSignatureBindable::AddComputeRootSignatureParam(RootSignatureParams* rootSignatureParams)
{

}

ResourceTargets& RootSignatureBindable::GetTargets()
{
	return m_targets;
}