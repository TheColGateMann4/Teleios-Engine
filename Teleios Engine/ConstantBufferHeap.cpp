#include "ConstantBufferHeap.h"
#include "Graphics.h"
#include "Macros/ErrorMacros.h"

unsigned int ConstantBufferHeap::RequestMoreSpace(Graphics& graphics, UINT resourceAlignedSize)
{
	m_bufferOffsets.push_back(m_combinedSize);
	m_combinedSize += resourceAlignedSize * graphics.GetBufferCount();

	return m_bufferOffsets.size() - 1;
}

void ConstantBufferHeap::Finish(Graphics& graphics)
{
	// creating constant buffer heap
	{
		HRESULT hr;

		D3D12_HEAP_PROPERTIES heapPropeties = {};
		heapPropeties.Type = D3D12_HEAP_TYPE_CUSTOM;
		heapPropeties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_COMBINE;
		heapPropeties.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;
		heapPropeties.VisibleNodeMask = 0;

		D3D12_RESOURCE_DESC resourceDesc = {};
		resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		resourceDesc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
		resourceDesc.Width = m_combinedSize;
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
			D3D12_RESOURCE_STATE_COMMON, //D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,
			nullptr,
			IID_PPV_ARGS(&pConstBufferHeap)
		));
	}
}

UINT64 ConstantBufferHeap::GetFrameBufferOffsetAtIndex(Graphics& graphics, unsigned int bufferIndex)
{
	return GetOffsetAtIndex(bufferIndex) + GetBufferSizeAtIndex(graphics, bufferIndex) * graphics.GetCurrentBufferIndex();
}

UINT64 ConstantBufferHeap::GetOffsetAtIndex(unsigned int bufferIndex)
{
	THROW_INTERNAL_ERROR_IF("Tried to access offset outside constant buffer heap", bufferIndex > m_bufferOffsets.size());

	return m_bufferOffsets.at(bufferIndex);
}

UINT64 ConstantBufferHeap::GetBufferSizeAtIndex(Graphics& graphics, unsigned int bufferIndex)
{
	UINT64 startOffset = GetOffsetAtIndex(bufferIndex);
	UINT64 endOffset = bufferIndex + 1 == m_bufferOffsets.size() ? m_combinedSize : GetOffsetAtIndex(bufferIndex + 1);

	return (endOffset - startOffset) / graphics.GetBufferCount();
}

D3D12_GPU_VIRTUAL_ADDRESS ConstantBufferHeap::GetBufferAddress(Graphics& graphics, unsigned int bufferIndex)
{
	D3D12_GPU_VIRTUAL_ADDRESS bufferAddress = pConstBufferHeap->GetGPUVirtualAddress();
	bufferAddress += GetFrameBufferOffsetAtIndex(graphics, bufferIndex);

	return bufferAddress;
}

ID3D12Resource* ConstantBufferHeap::Get()
{
	return pConstBufferHeap.Get();
}

void ConstantBufferHeap::Update(Graphics& graphics)
{
	unsigned int currFrameIndex = graphics.GetCurrentBufferIndex();

	for(unsigned int staticResourceIndex = 0; staticResourceIndex < m_staticResourcesToUpdate.size(); staticResourceIndex++)
	{
		auto& staticResourceToUpdate = m_staticResourcesToUpdate.at(staticResourceIndex);

		if(staticResourceToUpdate.frameIndex == currFrameIndex)
		{
			if (staticResourceToUpdate.alreadyUpdatedFirstTime)
			{
				m_staticResourcesToUpdate.erase(m_staticResourcesToUpdate.begin() + staticResourceIndex);
				staticResourceIndex--;

				continue;
			}
			else
			{
				staticResourceToUpdate.alreadyUpdatedFirstTime = true;
			}
		}

		UpdateResource(graphics, staticResourceToUpdate.resourceIndex, staticResourceToUpdate.data, staticResourceToUpdate.dataSize);
	}
}

void ConstantBufferHeap::UpdateResource(Graphics& graphics, unsigned int bufferIndex, void* data, size_t size)
{
	UINT64 bufferStartingOffset = GetFrameBufferOffsetAtIndex(graphics, bufferIndex);
	UINT64 bufferSize = GetBufferSizeAtIndex(graphics, bufferIndex);

	THROW_INTERNAL_ERROR_IF("Tried to update buffer as larger than it is", size > bufferSize);

	{
		HRESULT hr;

		D3D12_RANGE readRange = {};
		readRange.Begin = 0;
		readRange.End = 0;

		D3D12_RANGE writeRange = {};
		writeRange.Begin = bufferStartingOffset;
		writeRange.End = bufferStartingOffset + bufferSize;

		void* pMappedData = nullptr;

		THROW_ERROR(pConstBufferHeap->Map(
			0,
			&readRange,
			&pMappedData
		));

		memcpy_s(static_cast<char*>(pMappedData) + bufferStartingOffset, bufferSize, data, size);

		pConstBufferHeap->Unmap(0, &writeRange);
	}
}

void ConstantBufferHeap::UpdateStaticResource(Graphics& graphics, unsigned int bufferIndex, void* data, size_t size)
{
	StatcResourceToUpdate resourceToUpdate = {};
	resourceToUpdate.resourceIndex = bufferIndex;
	resourceToUpdate.data = data;
	resourceToUpdate.dataSize = size;
	resourceToUpdate.alreadyUpdatedFirstTime = false;
	resourceToUpdate.frameIndex = graphics.GetCurrentBufferIndex();

	m_staticResourcesToUpdate.push_back(resourceToUpdate);
}