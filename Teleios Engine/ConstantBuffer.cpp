#include "ConstantBuffer.h"
#include "Macros/ErrorMacros.h"
#include "Graphics.h"
#include "CommandList.h"
#include "ConstantBufferHeap.h"

ConstantBuffer::ConstantBuffer(Graphics& graphics, const DynamicConstantBuffer::ConstantBufferLayout& layout, std::vector<TargetSlotAndShader> targets)
	:
	RootSignatureBindable(targets)
{

}

void ConstantBuffer::BindToCommandList(Graphics& graphics, CommandList* commandList)
{
	commandList->SetGraphicsConstBufferView(graphics, this);
}

void ConstantBuffer::BindToRootSignature(Graphics& graphics, RootSignature* rootSignature)
{
	rootSignature->AddConstBufferViewParameter(this);
}

D3D12_GPU_VIRTUAL_ADDRESS ConstantBuffer::GetGPUAddress(Graphics& graphics) const
{
	return graphics.GetConstantBufferHeap().GetBufferAddress(graphics, resourceIndexInHeap);
}

void ConstantBuffer::InternalUpdate(Graphics& graphics, void* data, size_t size)
{
	graphics.GetConstantBufferHeap().UpdateResource(graphics, resourceIndexInHeap, data, size);
}

NonCachedConstantBuffer::NonCachedConstantBuffer(Graphics& graphics, DynamicConstantBuffer::ConstantBufferLayout& layout, std::vector<TargetSlotAndShader> targets)
	:
	ConstantBuffer(graphics, layout.GetFinished(), targets),
	m_layout(layout)
{
	// requesting size on non static heao
	resourceIndexInHeap = graphics.GetConstantBufferHeap().RequestMoreSpace(graphics, layout.GetSize());
}

void NonCachedConstantBuffer::Update(Graphics& graphics, void* data, size_t size)
{
	ConstantBuffer::InternalUpdate(graphics, data, size);
}

CachedConstantBuffer::CachedConstantBuffer(Graphics& graphics, DynamicConstantBuffer::ConstantBufferData& data, std::vector<TargetSlotAndShader> targets, bool frequentlyUpdated)
	:
	ConstantBuffer(graphics, data.GetLayout(), targets),
	m_data(data),
	m_frequentlyUpdated(frequentlyUpdated)
{
	// if resource is frequently updated we will keep it in regular buffer
	if(m_frequentlyUpdated)
		resourceIndexInHeap = graphics.GetConstantBufferHeap().RequestMoreSpace(graphics, data.GetLayout().GetSize());
	else
		resourceIndexInHeap = graphics.GetConstantBufferHeap().RequestMoreStaticSpace(graphics, data.GetLayout().GetSize());
}

void CachedConstantBuffer::Initialize(Graphics& graphics)
{
	// since we already pass the data to cached constant buffer, we surely want it updated on gpu side
	Update(graphics);
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

DynamicConstantBuffer::ConstantBufferData& CachedConstantBuffer::GetData()
{
	return m_data;
}

void CachedConstantBuffer::DrawImguiProperties(Graphics& graphics)
{
	if (m_data.DrawImguiProperties())
		Update(graphics);
}