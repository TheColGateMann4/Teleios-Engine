#include "DescriptorHeapBindable.h"

#include"Graphics/Core/Graphics.h"
#include"Graphics/Core/CommandList.h"
#include"Graphics/Core/RootSignature.h"

#include "Graphics/Core/ResourceList.h"

DescriptorHeapBindable::DescriptorHeapBindable(std::vector<TargetSlotAndShader> targets)
	:
	RootSignatureBindable(std::move(targets))
{

}

std::shared_ptr<DescriptorHeapBindable> DescriptorHeapBindable::GetResource(std::vector<TargetSlotAndShader> targets)
{
	return ResourceList::GetResource<DescriptorHeapBindable>(std::move(targets));
}

std::string DescriptorHeapBindable::GetIdentifier(std::vector<TargetSlotAndShader> targets)
{
	std::string result = {};

	for (const auto& target : targets)
	{
		result += std::to_string(target.slot);
		result += std::to_string(static_cast<unsigned int>(target.target));
		result += "#";
	}

	return result;
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