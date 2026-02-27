#include "DescriptorHeapBindable.h"

#include"Graphics/Core/Graphics.h"
#include"Graphics/Core/CommandList.h"
#include"Graphics/Core/RootSignature.h"

DescriptorHeapBindable::DescriptorHeapBindable(std::vector<TargetSlotAndShader> targets)
	:
	RootSignatureBindable(std::move(targets))
{

}

void DescriptorHeapBindable::BindToRootSignature(RootSignatureParams* rootSignatureParams)
{
	rootSignatureParams->AddDescriptorTableParameter(this);
}

void DescriptorHeapBindable::BindToCommandList(Graphics& graphics, CommandList* commandList)
{
	commandList->SetGraphicsDescriptorTable(graphics, this);
}

BindableType DescriptorHeapBindable::GetBindableType() const
{
	return BindableType::bindable_descriptorHeapBindable;
}

RootSignatureBindableType DescriptorHeapBindable::GetRootSignatureBindableType() const
{
	return RootSignatureBindableType::rootSignature_DescriptorTable;
}

D3D12_GPU_DESCRIPTOR_HANDLE DescriptorHeapBindable::GetDescriptorHeapGPUHandle() const
{
	return m_descriptorHandle;
}

void DescriptorHeapBindable::Initialize(Graphics& graphics, DescriptorHeap::DescriptorInfo descriptorInfo, unsigned int descriptorNum)
{

}

void DescriptorHeapBindable::Initialize(Graphics& graphics)
{
	// getting start address of descriptor heap to bind it whole
	m_descriptorHandle = graphics.GetDescriptorHeap().GetGPUHandle();
}

DescriptorType DescriptorHeapBindable::GetDescriptorType() const
{
	return DescriptorType::descriptor_SRV;
}