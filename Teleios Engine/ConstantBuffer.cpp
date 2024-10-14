#include "ConstantBuffer.h"
#include "Macros/ErrorMacros.h"
#include "Graphics.h"

ConstantBuffer::ConstantBuffer(Graphics& graphics, const DynamicConstantBuffer::ConstantBufferLayout& layout, TargetShader target, UINT slot)
	:
	m_target(target),
	m_slot(slot)
{
	HRESULT hr;

	// creating resource
	{
		D3D12_HEAP_PROPERTIES heapPropeties = {};
		heapPropeties.Type = D3D12_HEAP_TYPE_CUSTOM;
		heapPropeties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;
		heapPropeties.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;
		heapPropeties.VisibleNodeMask = 0;

		D3D12_RESOURCE_DESC resourceDesc = {};
		resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		resourceDesc.Alignment = 0;
		resourceDesc.Width = layout.GetSize();
		resourceDesc.Height = 1;
		resourceDesc.DepthOrArraySize = 1;
		resourceDesc.MipLevels = 1;
		resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
		resourceDesc.SampleDesc.Count = 1;
		resourceDesc.SampleDesc.Quality = 0;
		resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

		THROW_ERROR(graphics.GetDevice()->CreateCommittedResource(
			&heapPropeties,
			D3D12_HEAP_FLAG_NONE,
			&resourceDesc,
			D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,
			nullptr,
			IID_PPV_ARGS(&pConstBuffer)
		));
	}

	//creating decriptor heap
	{
		D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc = {};
		descriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		descriptorHeapDesc.NumDescriptors = 1;
		descriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		descriptorHeapDesc.NodeMask = 0;

		THROW_ERROR(graphics.GetDevice()->CreateDescriptorHeap(&descriptorHeapDesc, IID_PPV_ARGS(&pDescriptorHeap)));
	}

	// creating view
	{
		D3D12_CONSTANT_BUFFER_VIEW_DESC constBufferDesc = {};
		constBufferDesc.BufferLocation = pConstBuffer->GetGPUVirtualAddress();
		constBufferDesc.SizeInBytes = layout.GetSize();

		THROW_INFO_ERROR(graphics.GetDevice()->CreateConstantBufferView(
			&constBufferDesc,
			pDescriptorHeap->GetCPUDescriptorHandleForHeapStart()
		));
	}
}

void ConstantBuffer::SetRootIndex(UINT rootIndex)
{
	m_initializedRootIndex = true;

	m_rootIndex = rootIndex;
}

UINT ConstantBuffer::GetRootIndex() const
{
	THROW_INTERNAL_ERROR_IF("Tried to get root index without setting it beforehand", !m_initializedRootIndex);

	return m_rootIndex;
}

TargetShader ConstantBuffer::GetTarget() const
{
	return m_target;
}

UINT ConstantBuffer::GetSlot() const
{
	return m_slot;
}

D3D12_GPU_VIRTUAL_ADDRESS ConstantBuffer::GetGPUAddress() const
{
	return pConstBuffer->GetGPUVirtualAddress();
}

NonCachedConstantBuffer::NonCachedConstantBuffer(Graphics& graphics, const DynamicConstantBuffer::ConstantBufferLayout& layout, TargetShader target, UINT slot)
	:
	ConstantBuffer(graphics, layout, target, slot),
	m_layout(layout)
{

}

void NonCachedConstantBuffer::Update(Graphics& graphics, void* data, size_t size)
{
	THROW_INTERNAL_ERROR_IF("data passed to ConstantBuffer::SetData is invalid", size > m_layout.GetSize() || data == nullptr);

	HRESULT hr;

	// passing data to constant buffer resource
	{
		D3D12_RANGE readRange = {};
		readRange.Begin = 0;
		readRange.End = 0;

		D3D12_RANGE writeRange = {};
		writeRange.Begin = 0;
		writeRange.End = size;

		void* pMappedData = nullptr;

		THROW_ERROR(pConstBuffer->Map(
			0,
			&readRange,
			&pMappedData
		));

		memcpy_s(pMappedData, m_layout.GetSize(), data, size);

		pConstBuffer->Unmap(0, &writeRange);
	}
}

CachedConstantBuffer::CachedConstantBuffer(Graphics& graphics, DynamicConstantBuffer::ConstantBufferData& data, TargetShader target, UINT slot)
	:
	ConstantBuffer(graphics, data.GetLayout(), target, slot),
	m_data(data)
{

}

void CachedConstantBuffer::Update(Graphics& graphics)
{
	HRESULT hr;

	unsigned int bufferSize = m_data.GetLayout().GetSize();

	// passing data to constant buffer resource
	{
		D3D12_RANGE readRange = {};
		readRange.Begin = 0;
		readRange.End = 0;

		D3D12_RANGE writeRange = {};
		writeRange.Begin = 0;
		writeRange.End = bufferSize;

		void* pMappedData = nullptr;

		THROW_ERROR(pConstBuffer->Map(
			0,
			&readRange,
			&pMappedData
		));

		memcpy_s(pMappedData, bufferSize, m_data.GetPtr(), bufferSize);

		pConstBuffer->Unmap(0, &writeRange);
	}
}

DynamicConstantBuffer::ConstantBufferData& CachedConstantBuffer::GetData()
{
	return m_data;
}