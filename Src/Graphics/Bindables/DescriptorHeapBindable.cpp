#include "DescriptorHeapBindable.h"

#include"Graphics/Core/Graphics.h"
#include"Graphics/Core/CommandList.h"
#include"Graphics/Core/RootSignature.h"

#include "Graphics/Core/ResourceList.h"

DescriptorHeapBindable::DescriptorHeapBindable(ResourceTargets targets)
	:
	RootSignatureBindable(std::move(targets))
{

}

std::shared_ptr<DescriptorHeapBindable> DescriptorHeapBindable::GetResource(ResourceTargets targets)
{
	return ResourceList::GetResource<DescriptorHeapBindable>(std::move(targets));
}

std::string DescriptorHeapBindable::GetIdentifier(ResourceTargets targets)
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

void DescriptorHeapBindable::AddGraphicsRootSignatureParam(RootSignatureParams* rootSignatureParams)
{
	for(const auto& target : GetTargets())
		rootSignatureParams->AddDescriptorTableParameter(this, target);
}

void DescriptorHeapBindable::BindToCommandListAsRootParam(Graphics& graphics, CommandList* commandList, const RootBinding& binding)
{
	commandList->SetGraphicsDescriptorTable(graphics, this, binding);
}

BindableType DescriptorHeapBindable::GetBindableType() const
{
	return BindableType::bindable_descriptorHeapBindable;
}

RootSignatureBindableType DescriptorHeapBindable::GetRootSignatureBindableType() const
{
	return RootSignatureBindableType::rootSignature_DescriptorTable;
}

D3D12_GPU_DESCRIPTOR_HANDLE DescriptorHeapBindable::GetDescriptorHeapGPUHandle(Graphics& graphics) const
{
	return graphics.GetDescriptorHeap().GetHandle().descriptorHeapGpuHandle;
}

D3D12_GPU_VIRTUAL_ADDRESS DescriptorHeapBindable::GetGPUAddress(Graphics& graphics) const
{
	// TODO: make diverged class for objects that don't own a resource but are RootParams
	THROW_INTERNAL_ERROR("Tried to get gpu address of root bind that doesn't own a resource");

	return {};
}

void DescriptorHeapBindable::Initialize(Graphics& graphics, DescriptorHeap::DescriptorInfo descriptorInfo, unsigned int descriptorNum)
{

}

void DescriptorHeapBindable::Initialize(Graphics& graphics)
{
	
}

DescriptorType DescriptorHeapBindable::GetDescriptorType() const
{
	return DescriptorType::descriptor_SRV;
}