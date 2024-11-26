#include "Bindable.h"
#include "Macros/ErrorMacros.h"


RootSignatureBindable::RootSignatureBindable(std::vector<TargetSlotAndShader> targets)
	:
	m_targets(targets)
{

}

void RootSignatureBindable::InternalInitialize(Graphics& graphics)
{
	if (m_initialized)
		return;

	Initialize(graphics);

	m_initialized = true;
}

void RootSignatureBindable::Initialize(Graphics& graphics)
{

}

D3D12_GPU_VIRTUAL_ADDRESS RootSignatureBindable::GetGPUAddress(Graphics& graphics) const
{
	THROW_INTERNAL_ERROR("Object doesn't support getting GPU virtual address");
}

D3D12_GPU_DESCRIPTOR_HANDLE RootSignatureBindable::GetDescriptorHeapGPUHandle(Graphics& graphics) const
{
	THROW_INTERNAL_ERROR("Object doesn't use GPU Descriptors");
}

std::vector<TargetSlotAndShader>& RootSignatureBindable::GetTargets()
{
	return m_targets;
}