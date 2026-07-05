#include "RootSignatureConstants.h"

#include "Graphics/Core/RootSignature.h"
#include "Graphics/Core/CommandList.h"

RootSignatureConstants::RootSignatureConstants(DynamicConstantBuffer::Data& data, ResourceTargets targets)
	:
	RootSignatureBindable(targets),
	m_data(std::move(data))
{

}

void RootSignatureConstants::AddGraphicsRootSignatureParam(RootSignatureParams* rootSignatureParams)
{
	for (const auto& target : GetTargets())
		rootSignatureParams->SetGraphicsRootConstants(this, target);
}

void RootSignatureConstants::BindToCommandListAsRootParam(Graphics& graphics, CommandList* commandList, const RootBinding& binding)
{
	commandList->SetRootConstants(graphics, this, binding);
}

BindableType RootSignatureConstants::GetBindableType() const
{
	return BindableType::bindable_rootSignatureConstants;
}

RootSignatureBindableType RootSignatureConstants::GetRootSignatureBindableType() const
{
	return RootSignatureBindableType::rootSignature_Constants;
}

D3D12_GPU_VIRTUAL_ADDRESS RootSignatureConstants::GetGPUAddress(Graphics& graphics) const
{
	// TODO: make diverged class for objects that don't own a resource but are RootParams
	THROW_INTERNAL_ERROR("Tried to get gpu address of root bind that doesn't own a resource");

	return {};
}

unsigned int RootSignatureConstants::GetNumValues() const
{
	return m_data.GetLayout().GetSize() / 4;
}

const void* RootSignatureConstants::GetDataPtr() const
{
	m_isUpdated = false;

	return m_data.GetPtr();
}

bool RootSignatureConstants::IsUpdated() const
{
	return m_isUpdated;
}

void RootSignatureConstants::SetUpdated(bool updated)
{
	m_isUpdated = updated;
}

DynamicConstantBuffer::Data& RootSignatureConstants::GetData()
{
	return m_data;
}