#include "VertexBuffer.h"
#include "Macros/ErrorMacros.h"
#include "Graphics.h"
#include "CommandList.h"

VertexBuffer::VertexBuffer(Graphics& graphics, void* pData, size_t numElements, size_t dataStride)
{
	size_t dataSize = numElements * dataStride;
	HRESULT hr;

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

	// passing data to vetex buffer resource
	{
		D3D12_RANGE readRange = {};
		readRange.Begin = 0;
		readRange.End = 0;
		
		D3D12_RANGE writeRange = {};
		writeRange.Begin = 0;
		writeRange.End = dataSize;

		void* pMappedData = nullptr;

		THROW_ERROR(pVertexBuffer->Map(
			0,
			&readRange,
			&pMappedData
		));

		memcpy_s(pMappedData, dataSize, pData, dataSize);

		pVertexBuffer->Unmap(0, &writeRange);
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