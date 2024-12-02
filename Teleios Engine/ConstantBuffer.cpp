#include "ConstantBuffer.h"
#include "Macros/ErrorMacros.h"
#include "Graphics.h"
#include "CommandList.h"
#include "ConstantBufferHeap.h"

ConstantBuffer::ConstantBuffer(Graphics& graphics, const DynamicConstantBuffer::ConstantBufferLayout& layout, std::vector<TargetSlotAndShader> targets)
	:
	RootSignatureBindable(targets)
{
	// reguesting more space in constant buffer heap
	resourceIndexInHeap = graphics.GetConstantBufferHeap().RequestMoreSpace(graphics, layout.GetSize());
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

}

void NonCachedConstantBuffer::Update(Graphics& graphics, void* data, size_t size)
{
	ConstantBuffer::InternalUpdate(graphics, data, size);
}

CachedConstantBuffer::CachedConstantBuffer(Graphics& graphics, DynamicConstantBuffer::ConstantBufferData& data, std::vector<TargetSlotAndShader> targets)
	:
	ConstantBuffer(graphics, data.GetLayout(), targets),
	m_data(data)
{

}

void CachedConstantBuffer::Initialize(Graphics& graphics)
{
	// since we already pass the data to cached constant buffer, we surely want it updated on gpu side
	Update(graphics);
}

void CachedConstantBuffer::Update(Graphics& graphics)
{
	graphics.GetConstantBufferHeap().UpdateStaticResource(graphics, resourceIndexInHeap, m_data.GetPtr(), m_data.GetLayout().GetSize());
}

DynamicConstantBuffer::ConstantBufferData& CachedConstantBuffer::GetData()
{
	return m_data;
}