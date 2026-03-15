#include "RootSignatureConstants.h"

#include "Graphics/Core/RootSignature.h"
#include "Graphics/Core/CommandList.h"

RootSignatureConstants::RootSignatureConstants(DynamicConstantBuffer::Data& data, std::vector<TargetSlotAndShader> targets)
	:
	RootParameterBinding(targets),
	m_data(std::move(data))
{

}

void RootSignatureConstants::BindToCommandList(Graphics& graphics, CommandList* commandList, TargetSlotAndShader& target)
{
	commandList->SetRootConstants(graphics, this, target);
}

void RootSignatureConstants::BindToRootSignature(RootSignatureParams* rootSignatureParams, TargetSlotAndShader& target)
{
	rootSignatureParams->SetGraphicsRootConstants(this, target);
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

BindableType RootSignatureConstants::GetBindableType() const
{
	return BindableType::bindable_rootSignatureConstants;
}

RootSignatureBindableType RootSignatureConstants::GetRootSignatureBindableType() const
{
	return RootSignatureBindableType::rootSignature_Constants;
}