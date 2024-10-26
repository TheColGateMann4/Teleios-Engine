#include "IndexBuffer.h"
#include "Macros/ErrorMacros.h"
#include "Graphics.h"

IndexBuffer::IndexBuffer(Graphics& graphics, void* pData, size_t dataSize, DXGI_FORMAT dataFormat)
{
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
            D3D12_RESOURCE_STATE_INDEX_BUFFER,
            nullptr,
            IID_PPV_ARGS(&pIndexBuffer)
        ));
    }

	// passing data to index buffer resource
	{
		D3D12_RANGE readRange = {};
		readRange.Begin = 0;
		readRange.End = 0;

		D3D12_RANGE writeRange = {};
		writeRange.Begin = 0;
		writeRange.End = dataSize;

		void* pMappedData = nullptr;

		THROW_ERROR(pIndexBuffer->Map(
			0,
			&readRange,
			&pMappedData
		));

		memcpy_s(pMappedData, dataSize, pData, dataSize);

        pIndexBuffer->Unmap(0, &writeRange);
	}

    // initializing vertex buffer view
    {
        m_indexBufferView.BufferLocation = pIndexBuffer->GetGPUVirtualAddress();
        m_indexBufferView.SizeInBytes = dataSize;
        m_indexBufferView.Format = dataFormat;
    }
}

IndexBuffer::IndexBuffer(Graphics& graphics, std::vector<unsigned int> indices)
	:
	IndexBuffer(graphics, indices.data(), indices.size() * sizeof(indices.at(0)), DXGI_FORMAT_R32_UINT)
{

}

IndexBuffer::IndexBuffer(Graphics& graphics, std::vector<unsigned short> indices)
    :
    IndexBuffer(graphics, indices.data(), indices.size() * sizeof(indices.at(0)), DXGI_FORMAT_R16_UINT)
{

}

const D3D12_INDEX_BUFFER_VIEW* IndexBuffer::Get() const
{
    return &m_indexBufferView;
}