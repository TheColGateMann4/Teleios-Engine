#include "ConstantBuffer.h"
#include "Macros/ErrorMacros.h"
#include "Graphics/Core/Graphics.h"
#include "Graphics/Core/CommandList.h"
#include "Graphics/Core/ConstantBufferHeap.h"

ConstantBuffer::ConstantBuffer(Graphics& graphics, const DynamicConstantBuffer::Layout& layout, std::vector<TargetSlotAndShader> targets)
	:
	RootParameterBinding(targets)
{

}

void ConstantBuffer::BindToCommandList(Graphics& graphics, CommandList* commandList, TargetSlotAndShader& target)
{
	commandList->SetGraphicsConstBufferView(graphics, this, target);
}

void ConstantBuffer::BindToRootSignature(RootSignature* rootSignature, TargetSlotAndShader& target)
{
	rootSignature->AddConstBufferViewParameter(this, target);
}

D3D12_GPU_VIRTUAL_ADDRESS ConstantBuffer::GetGPUAddress(Graphics& graphics) const
{
	return graphics.GetConstantBufferHeap().GetBufferAddress(graphics, resourceIndexInHeap);
}

BindableType ConstantBuffer::GetBindableType() const
{
	return BindableType::bindable_constantBuffer;
}

RootSignatureBindableType ConstantBuffer::GetRootSignatureBindableType() const
{
	return RootSignatureBindableType::rootSignature_CBV;
}

void ConstantBuffer::InternalUpdate(Graphics& graphics, void* data, size_t size)
{
	graphics.GetConstantBufferHeap().UpdateResource(graphics, resourceIndexInHeap, data, size);
}

NonCachedConstantBuffer::NonCachedConstantBuffer(Graphics& graphics, DynamicConstantBuffer::Layout& layout, std::vector<TargetSlotAndShader> targets)
	:
	ConstantBuffer(graphics, layout.GetFinished(), targets),
	m_layout(std::move(layout))
{
	// requesting size on non static heao
	resourceIndexInHeap = graphics.GetConstantBufferHeap().RequestMoreSpace(graphics, m_layout.GetSize());
}

void NonCachedConstantBuffer::Update(Graphics& graphics, void* data, size_t size)
{
	ConstantBuffer::InternalUpdate(graphics, data, size);
}

BindableType NonCachedConstantBuffer::GetBindableType() const
{
	return BindableType::bindable_nonCachedConstantBuffer;
}

CachedConstantBuffer::CachedConstantBuffer(Graphics& graphics, DynamicConstantBuffer::Data& data, std::vector<TargetSlotAndShader> targets, bool frequentlyUpdated)
	:
	ConstantBuffer(graphics, data.GetLayout(), targets),
	m_data(std::move(data)),
	m_frequentlyUpdated(frequentlyUpdated)
{
	// if resource is frequently updated we will keep it in regular buffer
	if(m_frequentlyUpdated)
		resourceIndexInHeap = graphics.GetConstantBufferHeap().RequestMoreSpace(graphics, m_data.GetLayout().GetSize());
	else
		resourceIndexInHeap = graphics.GetConstantBufferHeap().RequestMoreStaticSpace(m_data.GetLayout().GetSize());
}

void CachedConstantBuffer::Update(Graphics& graphics)
{
	if (m_frequentlyUpdated)
		graphics.GetConstantBufferHeap().UpdateFrequentlyUpdatedStaticResource(graphics, resourceIndexInHeap, m_data.GetPtr(), m_data.GetLayout().GetSize());
	else
		graphics.GetConstantBufferHeap().UpdateStaticResource(graphics, resourceIndexInHeap, m_data.GetPtr(), m_data.GetLayout().GetSize());
}

D3D12_GPU_VIRTUAL_ADDRESS CachedConstantBuffer::GetGPUAddress(Graphics& graphics) const
{
	if (m_frequentlyUpdated)
		return graphics.GetConstantBufferHeap().GetBufferAddress(graphics, resourceIndexInHeap);
	else
		return graphics.GetConstantBufferHeap().GetStaticBufferAddress(resourceIndexInHeap);

}

BindableType CachedConstantBuffer::GetBindableType() const
{
	return BindableType::bindable_cachedConstantBuffer;
}

DynamicConstantBuffer::Data& CachedConstantBuffer::GetData()
{
	return m_data;
}

void CachedConstantBuffer::DrawImguiProperties(Graphics& graphics)
{
	if (m_data.DrawImguiProperties())
		Update(graphics);
}

TempConstantBuffer::TempConstantBuffer(Graphics& graphics, DynamicConstantBuffer::Data& data, std::vector<TargetSlotAndShader> targets, bool frequentlyUpdated)
	:
	ConstantBuffer(graphics, data.GetLayout(), targets),
	m_data(std::move(data))
{
	resourceIndexInHeap = graphics.GetConstantBufferHeap().GetNextTempIndex(m_data.GetLayout().GetSize());

	Update(graphics);
}

void TempConstantBuffer::Update(Graphics& graphics)
{
	graphics.GetConstantBufferHeap().UpdateTempResource(graphics, resourceIndexInHeap, m_data.GetPtr(), m_data.GetLayout().GetSize());
}

void TempConstantBuffer::BindToCommandList(Graphics& graphics, CommandList* commandList, TargetSlotAndShader& target)
{
	commandList->SetComputeConstBufferView(graphics, this, target);
}

void TempConstantBuffer::BindToComputeRootSignature(RootSignature* rootSignature, TargetSlotAndShader& target)
{
	rootSignature->AddConstBufferViewParameter(this, target);
}

D3D12_GPU_VIRTUAL_ADDRESS TempConstantBuffer::GetGPUAddress(Graphics& graphics) const
{
	return graphics.GetConstantBufferHeap().GetTempBufferAddress(resourceIndexInHeap);
}

DynamicConstantBuffer::Data& TempConstantBuffer::GetData()
{
	return m_data;
}