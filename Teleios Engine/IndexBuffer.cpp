#include "IndexBuffer.h"
#include "Macros/ErrorMacros.h"
#include "Graphics.h"
#include "Pipeline.h"
#include "CommandList.h"

#include "BindableResourceList.h"

IndexBuffer::IndexBuffer(Graphics& graphics, void* pData, size_t indexCount, DXGI_FORMAT dataFormat)
    :
	m_dataFormat(dataFormat)
{
    THROW_OBJECT_STATE_ERROR_IF("Haven't handled other indice formats", m_dataFormat != DXGI_FORMAT_R32_UINT && m_dataFormat != DXGI_FORMAT_R16_UINT);

    uint8_t structureSize = m_dataFormat == DXGI_FORMAT_R32_UINT ? 4 : 2; // 4bytes for 32bits and 2bytes for 16bits

	CreateResource(graphics, indexCount, structureSize, pIndexBuffer);

	CreateResource(graphics, indexCount, structureSize, pUploadResource, true);

	// passing data to vetex buffer resource
	UpdateBufferData(graphics, pData, indexCount, structureSize);
}

IndexBuffer::IndexBuffer(Graphics& graphics, std::vector<unsigned int> indices)
	:
	IndexBuffer(graphics, indices.data(), indices.size(), DXGI_FORMAT_R32_UINT)
{

}

IndexBuffer::IndexBuffer(Graphics& graphics, std::vector<unsigned short> indices)
    :
    IndexBuffer(graphics, indices.data(), indices.size(), DXGI_FORMAT_R16_UINT)
{

}

std::shared_ptr<IndexBuffer> IndexBuffer::GetBindableResource(Graphics& graphics, std::string identifier, std::vector<unsigned int> indices)
{
	return BindableResourceList::GetBindableResourceByID<IndexBuffer>(graphics, "IndexBuffer#" + identifier, indices);
}

std::shared_ptr<IndexBuffer> IndexBuffer::GetBindableResource(Graphics& graphics, std::string identifier, std::vector<unsigned short> indices)
{
	return BindableResourceList::GetBindableResourceByID<IndexBuffer>(graphics, "IndexBuffer#" + identifier, indices);
}

void IndexBuffer::BindToCopyPipelineIfNeeded(Pipeline& pipeline)
{
	if (!pUploadResource) // if uploadResource ptr is not holding value then we don't want to update
		return;

	pipeline.AddBufferToCopyPipeline(this);
}

void IndexBuffer::CopyResources(Graphics& graphics, CommandList* copyCommandList)
{
	if (!pUploadResource) // shared resource got already updated by another object
		return;

	copyCommandList->SetResourceState(graphics, pIndexBuffer.Get(), D3D12_RESOURCE_STATE_INDEX_BUFFER, D3D12_RESOURCE_STATE_COPY_DEST);
	copyCommandList->SetResourceState(graphics, pUploadResource.Get(), D3D12_RESOURCE_STATE_INDEX_BUFFER, D3D12_RESOURCE_STATE_COPY_SOURCE);

	copyCommandList->CopyResource(graphics, pIndexBuffer.Get(), pUploadResource.Get());

	copyCommandList->SetResourceState(graphics, pIndexBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_INDEX_BUFFER);
	copyCommandList->SetResourceState(graphics, pUploadResource.Get(), D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_INDEX_BUFFER);

	graphics.GetFrameResourceDeleter()->DeleteResource(graphics, std::move(pUploadResource));
	pUploadResource.Reset();
}

void IndexBuffer::BindToCommandList(Graphics& graphics, CommandList* commandList)
{
    commandList->SetIndexBuffer(graphics, this);
}

void IndexBuffer::Update(Graphics& graphics, void* pData, size_t numElements, size_t structureSize)
{
	// if sizes are not the same, we will be creating new GPU resource.
	if (m_bufferSize != numElements * structureSize)
	{
		graphics.GetFrameResourceDeleter()->DeleteResource(graphics, std::move(pIndexBuffer));
		CreateResource(graphics, numElements, structureSize, pIndexBuffer);
	}

	CreateResource(graphics, numElements, structureSize, pUploadResource, true);

	UpdateBufferData(graphics, pData, numElements, structureSize);
}

const D3D12_INDEX_BUFFER_VIEW* IndexBuffer::Get() const
{
    return &m_indexBufferView;
}

size_t IndexBuffer::GetIndexCount() const
{
    return m_indexCount;
}

void IndexBuffer::UpdateBufferData(Graphics& graphics, void* pData, size_t numElements, size_t structureSize)
{
	HRESULT hr;

	// updating data
	{
		D3D12_RANGE readRange = {};
		readRange.Begin = 0;
		readRange.End = 0;

		D3D12_RANGE writeRange = {};
		writeRange.Begin = 0;
		writeRange.End = m_bufferSize;

		void* pMappedData = nullptr;

		THROW_ERROR(pUploadResource->Map(
			0,
			&readRange,
			&pMappedData
		));

		memcpy_s(pMappedData, m_bufferSize, pData, m_bufferSize);

		pUploadResource->Unmap(0, &writeRange);
	}
}

void IndexBuffer::CreateResource(Graphics& graphics, size_t numElements, size_t structureSize, Microsoft::WRL::ComPtr<ID3D12Resource>& resultResource, bool isUploadResource)
{
	HRESULT hr;

	m_bufferSize = numElements * structureSize;
	m_indexCount = numElements;

	// initializing vertex buffer resource
	{
		D3D12_HEAP_PROPERTIES heapPropeties = {};
		heapPropeties.Type = D3D12_HEAP_TYPE_CUSTOM;
		heapPropeties.CPUPageProperty = isUploadResource ? D3D12_CPU_PAGE_PROPERTY_WRITE_BACK : D3D12_CPU_PAGE_PROPERTY_NOT_AVAILABLE;
		heapPropeties.MemoryPoolPreference = isUploadResource ? D3D12_MEMORY_POOL_L0 : D3D12_MEMORY_POOL_L1;
		heapPropeties.VisibleNodeMask = 0;

		D3D12_RESOURCE_DESC resourceDesc = {};
		resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		resourceDesc.Alignment = 0;
		resourceDesc.Width = m_bufferSize;
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
			D3D12_RESOURCE_STATE_COMMON, //D3D12_RESOURCE_STATE_INDEX_BUFFER,
			nullptr,
			IID_PPV_ARGS(&resultResource)
		));
	}

	// initializing vertex buffer view
	if(!isUploadResource)
	{
		m_indexBufferView.BufferLocation = resultResource->GetGPUVirtualAddress();
		m_indexBufferView.SizeInBytes = m_bufferSize;
		m_indexBufferView.Format = m_dataFormat;
	}
}