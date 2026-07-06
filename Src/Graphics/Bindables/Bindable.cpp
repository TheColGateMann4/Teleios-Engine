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
	THROW_INTERNAL_ERROR("Called BindToComputeCommandList that wasn't overloaded");
}

void PipelineStateBindable::AddComputePipelineStateParam(Graphics& graphics, ComputePipelineStateParams* params)
{
	THROW_INTERNAL_ERROR("Called AddComputePipelineStateParam that wasn't overloaded");
}

RootSignatureBindable::RootSignatureBindable(ResourceTargets targets)
	:
	m_targets(targets)
{

}

const ResourceTargets& RootSignatureBindable::GetTargets() const
{
	return m_targets;
}