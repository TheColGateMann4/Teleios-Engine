#include "DescriptorHeapBindable.h"

#include"Graphics/Core/Graphics.h"
#include"Graphics/Core/CommandList.h"
#include"Graphics/Core/RootSignature.h"

DescriptorHeapBindable::DescriptorHeapBindable(Graphics& graphics, std::vector<TargetSlotAndShader> targets)
	:
	RootSignatureBindable(std::move(targets))
{
	// getting start address of descriptor heap to bind it whole
	m_descriptorHandle = graphics.GetDescriptorHeap().GetGPUHandle();
}

void DescriptorHeapBindable::BindToRootSignature(RootSignature* rootSignature)
{
	rootSignature->AddDescriptorTableParameter(this);
}

void DescriptorHeapBindable::BindToCommandList(Graphics& graphics, CommandList* commandList)
{
	commandList->SetGraphicsDescriptorTable(graphics, this);
}

D3D12_GPU_DESCRIPTOR_HANDLE DescriptorHeapBindable::GetDescriptorHeapGPUHandle() const
{
	return m_descriptorHandle;
}