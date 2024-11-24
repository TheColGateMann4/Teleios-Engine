#include "ConstantBuffer.h"
#include "Macros/ErrorMacros.h"
#include "Graphics.h"
#include "CommandList.h"

ConstantBuffer::ConstantBuffer(Graphics& graphics, const DynamicConstantBuffer::ConstantBufferLayout& layout, std::vector<TargetSlotAndShader> targets)
	:
	RootSignatureBindable(targets)
{
	HRESULT hr;
	unsigned int numberOfBuffers = graphics.GetBufferCount();

	pConstBuffers.resize(numberOfBuffers);

	// creating resource
	for (unsigned int bufferIndex = 0; bufferIndex < numberOfBuffers; bufferIndex++)
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

		// this is very incorrect practice since creating many different commited resources for one purpose is bad practice. It is only temporary solution
		THROW_ERROR(graphics.GetDevice()->CreateCommittedResource(
			&heapPropeties,
			D3D12_HEAP_FLAG_NONE,
			&resourceDesc,
			D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,
			nullptr,
			IID_PPV_ARGS(&pConstBuffers.at(bufferIndex))
		));
	}
	}

void ConstantBuffer::BindToCommandList(Graphics& graphics, CommandList* commandList)
{
	commandList->SetConstBufferView(graphics, this);
}

void ConstantBuffer::BindToRootSignature(Graphics& graphics, RootSignature* rootSignature)
{
	rootSignature->AddConstBufferViewParameter(this);
}

D3D12_GPU_VIRTUAL_ADDRESS ConstantBuffer::GetGPUAddress(Graphics& graphics) const
{
	return pConstBuffers.at(graphics.GetCurrentBufferIndex())->GetGPUVirtualAddress();
}

void ConstantBuffer::InternalUpdate(Graphics& graphics, void* data, size_t size)
{
	Microsoft::WRL::ComPtr<ID3D12Resource> pConstBuffer = pConstBuffers.at(graphics.GetCurrentBufferIndex());

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

		memcpy_s(pMappedData, size, data, size);

		pConstBuffer->Unmap(0, &writeRange);
	}
}

NonCachedConstantBuffer::NonCachedConstantBuffer(Graphics& graphics, DynamicConstantBuffer::ConstantBufferLayout& layout, std::vector<TargetSlotAndShader> targets)
	:
	ConstantBuffer(graphics, layout.GetFinished(), targets),
	m_layout(layout)
{

}

void NonCachedConstantBuffer::Update(Graphics& graphics, void* data, size_t size)
{
	THROW_OBJECT_STATE_ERROR_IF("Data is invalid", size > m_layout.GetSize() || data == nullptr);

	ConstantBuffer::InternalUpdate(graphics, data, size);
}

CachedConstantBuffer::CachedConstantBuffer(Graphics& graphics, DynamicConstantBuffer::ConstantBufferData& data, std::vector<TargetSlotAndShader> targets)
	:
	ConstantBuffer(graphics, data.GetLayout(), targets),
	m_data(data),
	m_firstBufferAlreadyUpdated(false),
	m_bufferUpdatedAtIndex(0),
	m_allBuffersUpdated(true)
{
	// since we already pass the data to cached constant buffer, we surely want it updated on gpu side
	Update(graphics);
}

void CachedConstantBuffer::Update(Graphics& graphics)
{
	m_bufferUpdatedAtIndex = graphics.GetCurrentBufferIndex();
	m_firstBufferAlreadyUpdated = false;
	m_allBuffersUpdated = false;
}

void CachedConstantBuffer::UpdateNextFrameResource(Graphics& graphics)
{
	// skip if all buffers are up to date
	if (m_allBuffersUpdated)
		return;

	bool currentFrameIndexIsSameAsUpdated = m_bufferUpdatedAtIndex == graphics.GetCurrentBufferIndex();

	// if we are at index where update call was made, and first buffer is updated then we are in next cycle. All are updated
	if (currentFrameIndexIsSameAsUpdated && m_firstBufferAlreadyUpdated)
	{
		m_allBuffersUpdated = true;
		m_firstBufferAlreadyUpdated = false;

		return;
	}

	ConstantBuffer::InternalUpdate(graphics, m_data.GetPtr(), m_data.GetLayout().GetSize());

	// if we are at index where update call was made and first buffer was not updated, then we should update it since its first frame where update occured
	if (currentFrameIndexIsSameAsUpdated && !m_firstBufferAlreadyUpdated)
		m_firstBufferAlreadyUpdated = true;
}

DynamicConstantBuffer::ConstantBufferData& CachedConstantBuffer::GetData()
{
	return m_data;
}