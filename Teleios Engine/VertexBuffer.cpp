#include "VertexBuffer.h"
#include "Macros/ErrorMacros.h"
#include "Graphics.h"
#include "CommandList.h"

#include "DynamicVertex.h"

#include "BindableResourceList.h"

VertexBuffer::VertexBuffer(Graphics& graphics, DynamicVertex::DynamicVertex& dynamicVertexBuffer)
	:
	VertexBuffer(graphics, dynamicVertexBuffer.GetData(), dynamicVertexBuffer.GetNumVertices(), dynamicVertexBuffer.GetLayout().GetSize())
{

}

VertexBuffer::VertexBuffer(Graphics& graphics, void* pData, size_t numElements, size_t dataStride)
{
	CreateResource(graphics, numElements * dataStride, dataStride);

	// passing data to vetex buffer resource
	UpdateBufferData(graphics, pData, numElements, dataStride);
}

std::shared_ptr<VertexBuffer> VertexBuffer::GetBindableResource(std::string identifier, Graphics& graphics, DynamicVertex::DynamicVertex& dynamicVertexBuffer)
{
	return BindableResourceList::GetBindableResourceByID<VertexBuffer>("VertexBuffer#" + identifier, graphics, dynamicVertexBuffer);
}

std::shared_ptr<VertexBuffer> VertexBuffer::GetBindableResource(std::string identifier, Graphics& graphics, void* pData, size_t numElements, size_t dataStride)
{
	return BindableResourceList::GetBindableResourceByID<VertexBuffer>("VertexBuffer#" + identifier, graphics, pData, numElements, dataStride);
}

void VertexBuffer::Update(Graphics& graphics, void* pData, size_t numElements, size_t dataStride)
{
	size_t newDataSize = numElements * dataStride;

	if(m_bufferSize != newDataSize)
		CreateResource(graphics, newDataSize, dataStride);

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

	THROW_ERROR(pVertexBuffer->Map(
		0,
		&readRange,
		&pMappedData
	));

	memcpy_s(pMappedData, m_bufferSize, pData, m_bufferSize);

	pVertexBuffer->Unmap(0, &writeRange);
}

void VertexBuffer::CreateResource(Graphics& graphics, size_t dataSize, size_t dataStride)
{
	HRESULT hr;

	m_bufferSize = dataSize;

	// initializing vertex buffer resource
	{
		D3D12_HEAP_PROPERTIES heapPropeties = {};
		heapPropeties.Type = D3D12_HEAP_TYPE_CUSTOM;
		heapPropeties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;
		heapPropeties.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;
		heapPropeties.VisibleNodeMask = 0;

		D3D12_RESOURCE_DESC resourceDesc = {};
		resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		resourceDesc.Alignment = 0;
		resourceDesc.Width = dataSize;
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
			IID_PPV_ARGS(&pVertexBuffer)
		));
	}

	// initializing vertex buffer view
	{
		m_vertexBufferView.BufferLocation = pVertexBuffer->GetGPUVirtualAddress();
		m_vertexBufferView.SizeInBytes = dataSize;
		m_vertexBufferView.StrideInBytes = dataStride;
	}
}

void VertexBuffer::BindToCommandList(Graphics& graphics, CommandList* commandList)
{
	commandList->SetVertexBuffer(graphics, this);
}

const D3D12_VERTEX_BUFFER_VIEW* VertexBuffer::Get() const
{
	return &m_vertexBufferView;
}