#include "Binding.h"

#include "Macros/ErrorMacros.h"

void CommandListBinding::BindToComputeCommandList(Graphics& graphics, CommandList* commandList, TargetSlotAndShader& target)
{

}

RootSignatureBinding::RootSignatureBinding(std::vector<TargetSlotAndShader> targets)
	:
	m_targets(targets)
{

}

void RootSignatureBinding::BindToComputeRootSignature(RootSignature* rootSignature, TargetSlotAndShader& target)
{

}

D3D12_GPU_VIRTUAL_ADDRESS RootSignatureBinding::GetGPUAddress(Graphics& graphics) const
{
	THROW_INTERNAL_ERROR("Object doesn't support getting GPU virtual address");
}

D3D12_GPU_DESCRIPTOR_HANDLE RootSignatureBinding::GetDescriptorHeapGPUHandle(Graphics& graphics) const
{
	THROW_INTERNAL_ERROR("Object doesn't use GPU Descriptors");
}

std::vector<TargetSlotAndShader>& RootSignatureBinding::GetTargets()
{
	return m_targets;
}

void RootSignatureBinding::Initialize(Graphics& graphics)
{

}

RootParameterBinding::RootParameterBinding(std::vector<TargetSlotAndShader> targets)
	:
	RootSignatureBinding(targets)
{

}