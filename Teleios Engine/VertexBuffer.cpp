#include "VertexBuffer.h"
#include "Macros/ErrorMacros.h"
#include "Graphics.h"

VertexBuffer::VertexBuffer(Graphics& graphics, void* pData, size_t dataSize, size_t dataStride)
{
	// initializing vertex buffer resource
	{
		HRESULT hr;

		D3D12_HEAP_PROPERTIES heapPropeties = {};
		heapPropeties.Type = D3D12_HEAP_TYPE_DEFAULT;
		heapPropeties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heapPropeties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
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
		m_vertexBufferView.SizeInBytes = dataSize * dataStride;
		m_vertexBufferView.StrideInBytes = dataStride;
	}
}


const D3D12_VERTEX_BUFFER_VIEW* VertexBuffer::Get() const
{
	return &m_vertexBufferView;
}