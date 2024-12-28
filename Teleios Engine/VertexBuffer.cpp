#include "VertexBuffer.h"
#include "Macros/ErrorMacros.h"
#include "Graphics.h"
#include "CommandList.h"

#include "Pipeline.h"

#include "DynamicVertex.h"

#include "BindableResourceList.h"

VertexBuffer::VertexBuffer(Graphics& graphics, DynamicVertex::DynamicVertex& dynamicVertexBuffer)
	:
	VertexBuffer(graphics, dynamicVertexBuffer.GetData(), dynamicVertexBuffer.GetNumVertices(), dynamicVertexBuffer.GetLayout().GetSize())
{

}

VertexBuffer::VertexBuffer(Graphics& graphics, void* pData, size_t numElements, size_t dataStride)
{
	CreateResource(graphics, numElements, dataStride, pVertexBuffer);

	CreateResource(graphics, numElements, dataStride, pUploadResource , true);

	// passing data to vetex buffer resource
	UpdateBufferData(graphics, pData, numElements, dataStride);
}

std::shared_ptr<VertexBuffer> VertexBuffer::GetBindableResource(Graphics& graphics, std::string identifier, DynamicVertex::DynamicVertex& dynamicVertexBuffer)
{
	return GetBindableResource(graphics, identifier, dynamicVertexBuffer.GetData(), dynamicVertexBuffer.GetNumVertices(), dynamicVertexBuffer.GetLayout().GetSize());
}

std::shared_ptr<VertexBuffer> VertexBuffer::GetBindableResource(Graphics& graphics, std::string identifier, void* pData, size_t numElements, size_t dataStride)
{
	return BindableResourceList::GetBindableResourceByID<VertexBuffer>(graphics, "VertexBuffer#" + identifier, pData, numElements, dataStride);
}

void VertexBuffer::BindToCopyPipelineIfNeeded(Pipeline& pipeline)
{
	if (!pUploadResource) // if uploadResource ptr is not holding value then we don't want to update
		return;

	pipeline.AddBufferToCopyPipeline(this);
}

void VertexBuffer::CopyResources(Graphics& graphics, CommandList* copyCommandList)
{
	if (!pUploadResource) // shared resource got already updated by another object
		return;

	copyCommandList->SetResourceState(graphics, pVertexBuffer.Get(), D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, D3D12_RESOURCE_STATE_COPY_DEST);
	copyCommandList->SetResourceState(graphics, pUploadResource.Get(), D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, D3D12_RESOURCE_STATE_COPY_SOURCE);

	copyCommandList->CopyResource(graphics, pVertexBuffer.Get(), pUploadResource.Get());

	copyCommandList->SetResourceState(graphics, pVertexBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
	copyCommandList->SetResourceState(graphics, pUploadResource.Get(), D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);

	graphics.GetFrameResourceDeleter()->DeleteResource(graphics, std::move(pUploadResource));
	pUploadResource.Reset();
}

void VertexBuffer::BindToCommandList(Graphics& graphics, CommandList* commandList)
{
	commandList->SetVertexBuffer(graphics, this);
}

void VertexBuffer::Update(Graphics& graphics, void* pData, size_t numElements, size_t dataStride)
{
	// if sizes are not the same, we will be creating new GPU resource.
	if (m_bufferSize != dataStride * numElements)
	{
		graphics.GetFrameResourceDeleter()->DeleteResource(graphics, std::move(pVertexBuffer));
		CreateResource(graphics, numElements, dataStride, pVertexBuffer);
	}

	CreateResource(graphics, numElements, dataStride, pUploadResource, true);

	UpdateBufferData(graphics, pData, numElements, dataStride);
}

void VertexBuffer::UpdateBufferData(Graphics& graphics, void* pData, size_t numElements, size_t dataStride)
{
	HRESULT hr;

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

void VertexBuffer::CreateResource(Graphics& graphics, size_t numElements, size_t dataStride, Microsoft::WRL::ComPtr<ID3D12Resource>& resultResource, bool isUploadResource)
{
	HRESULT hr;

	m_bufferSize = numElements * dataStride;

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
			D3D12_RESOURCE_STATE_COMMON, // D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,
			nullptr,
			IID_PPV_ARGS(&resultResource)
		));
	}

	// initializing vertex buffer view only for GPU memory space resource
	if(!pUploadResource)
	{
		m_vertexBufferView.BufferLocation = resultResource->GetGPUVirtualAddress();
		m_vertexBufferView.SizeInBytes = m_bufferSize;
		m_vertexBufferView.StrideInBytes = dataStride;
	}
}

const D3D12_VERTEX_BUFFER_VIEW* VertexBuffer::Get() const
{
	return &m_vertexBufferView;
}