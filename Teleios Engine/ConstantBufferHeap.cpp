#include "ConstantBufferHeap.h"
#include "Macros/ErrorMacros.h"
#include "Graphics.h"
#include "CommandList.h"

unsigned int ConstantBufferHeap::GetNextTempIndex(UINT resourceAlignedSize)
{
	THROW_INTERNAL_ERROR_IF("Temp buffer size was higher than 256", resourceAlignedSize > 256);
	THROW_INTERNAL_ERROR_IF("Ran out of space for temp buffers", m_numTempBuffersUsed == m_numberOfTempBuffers);

	m_numTempBuffersUsed++;

	return m_numTempBuffersUsed - 1;
}

unsigned int ConstantBufferHeap::RequestMoreStaticSpace(UINT resourceAlignedSize)
{
	THROW_OBJECT_STATE_ERROR_IF("Tried to Request more space when constant buffer heap is finished", m_finished);

	m_staticBufferOffsets.push_back(m_combinedSizeStaticBuffer);
	m_combinedSizeStaticBuffer += resourceAlignedSize;

	return m_staticBufferOffsets.size() - 1;
}

unsigned int ConstantBufferHeap::RequestMoreSpace(Graphics& graphics, UINT resourceAlignedSize)
{
	THROW_OBJECT_STATE_ERROR_IF("Tried to Request more space when constant buffer heap is finished", m_finished);

	m_bufferOffsets.push_back(m_combinedSize);
	m_combinedSize += resourceAlignedSize * graphics.GetBufferCount();

	return m_bufferOffsets.size() - 1;
}

void ConstantBufferHeap::Finish(Graphics& graphics)
{
	THROW_OBJECT_STATE_ERROR_IF("Tried to finish  when constant buffer heap is finished", m_finished);

	HRESULT hr;

	D3D12_HEAP_PROPERTIES heapPropeties = {};
	heapPropeties.Type = D3D12_HEAP_TYPE_CUSTOM;
	heapPropeties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_COMBINE;
	heapPropeties.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;
	heapPropeties.VisibleNodeMask = 0;

	D3D12_RESOURCE_DESC resourceDesc = {};
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resourceDesc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
	resourceDesc.Width = m_combinedSize + m_numberOfTempBuffers * 256;
	resourceDesc.Height = 1;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.MipLevels = 1;
	resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.SampleDesc.Quality = 0;
	resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	// creating constant buffer of non static resources
	{

		THROW_ERROR(graphics.GetDevice()->CreateCommittedResource(
			&heapPropeties,
			D3D12_HEAP_FLAG_NONE,
			&resourceDesc,
			D3D12_RESOURCE_STATE_COMMON, //D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,
			nullptr,
			IID_PPV_ARGS(&pBufferHeap)
		));
	}

	// creating "static" constant buffer resource
	{
		heapPropeties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_NOT_AVAILABLE; // cpu won't be able to modify this varible
		heapPropeties.MemoryPoolPreference = D3D12_MEMORY_POOL_L1; // L1 memory pool is strictly for GPU

		resourceDesc.Width = m_combinedSizeStaticBuffer; // setting size to the one for static resources

		THROW_ERROR(graphics.GetDevice()->CreateCommittedResource(
			&heapPropeties,
			D3D12_HEAP_FLAG_NONE,
			&resourceDesc,
			D3D12_RESOURCE_STATE_COMMON, //D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,
			nullptr,
			IID_PPV_ARGS(&pStaticBufferHeap)
		));
	}

	m_finished = true;
}

UINT64 ConstantBufferHeap::GetOffsetOfFrameBufferAtIndex(Graphics& graphics, unsigned int bufferIndex)
{
	return GetOffsetAtIndex(bufferIndex) + GetBufferSizeAtIndex(graphics, bufferIndex) * graphics.GetCurrentBufferIndex();
}

UINT64 ConstantBufferHeap::GetOffsetOfTempBufferAtIndex(unsigned int bufferIndex)
{
	return m_combinedSize + bufferIndex * 256;
}

UINT64 ConstantBufferHeap::GetOffsetOfStaticBufferAtIndex(unsigned int bufferIndex)
{
	THROW_INTERNAL_ERROR_IF("Tried to access offset outside constant buffer heap", bufferIndex > m_staticBufferOffsets.size());

	return m_staticBufferOffsets.at(bufferIndex);
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

UINT64 ConstantBufferHeap::GetStaticBufferSizeAtIndex(Graphics& graphics, unsigned int bufferIndex)
{
	UINT64 startOffset = GetOffsetOfStaticBufferAtIndex(bufferIndex);
	UINT64 endOffset = bufferIndex + 1 == m_staticBufferOffsets.size() ? m_combinedSizeStaticBuffer : GetOffsetOfStaticBufferAtIndex(bufferIndex + 1);

	return (endOffset - startOffset);
}

D3D12_GPU_VIRTUAL_ADDRESS ConstantBufferHeap::GetTempBufferAddress(unsigned int bufferIndex)
{
	THROW_INTERNAL_ERROR_IF("Tried to access buffer outside of range", bufferIndex > m_numberOfTempBuffers);

	D3D12_GPU_VIRTUAL_ADDRESS bufferAddress = pBufferHeap->GetGPUVirtualAddress();
	bufferAddress += GetOffsetOfTempBufferAtIndex(bufferIndex);

	return bufferAddress;
}

D3D12_GPU_VIRTUAL_ADDRESS ConstantBufferHeap::GetBufferAddress(Graphics& graphics, unsigned int bufferIndex)
{
	D3D12_GPU_VIRTUAL_ADDRESS bufferAddress = pBufferHeap->GetGPUVirtualAddress();
	bufferAddress += GetOffsetOfFrameBufferAtIndex(graphics, bufferIndex);

	return bufferAddress;
}

D3D12_GPU_VIRTUAL_ADDRESS ConstantBufferHeap::GetStaticBufferAddress(unsigned int bufferIndex)
{
	D3D12_GPU_VIRTUAL_ADDRESS bufferAddress = pStaticBufferHeap->GetGPUVirtualAddress();
	bufferAddress += GetOffsetOfStaticBufferAtIndex(bufferIndex);

	return bufferAddress;
}

void ConstantBufferHeap::CopyResources(Graphics& graphics, CommandList* copyCommandList)
{
	THROW_OBJECT_STATE_ERROR_IF("Buffer was not finished", !m_finished);

	bool isStaticBufferUsed = false;

	for (size_t updateResourceDataIndex = 0; updateResourceDataIndex < m_uploadResources.size(); updateResourceDataIndex++)
	{
		UploadResource& uploadData = m_uploadResources.at(updateResourceDataIndex);

		if (uploadData.updatedAtFrameIndex != graphics.GetCurrentBufferIndex())
			continue;

		if(uploadData.alreadyUpdated)
		{
			m_uploadResources.erase(m_uploadResources.begin() + updateResourceDataIndex);
			continue;
		}

		if(!isStaticBufferUsed)
		{
			copyCommandList->SetResourceState(graphics, pStaticBufferHeap.Get(), D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, D3D12_RESOURCE_STATE_COPY_DEST);
			isStaticBufferUsed = true;
		}

		UINT64 bufferStartingOffset = GetOffsetOfStaticBufferAtIndex(uploadData.staticResourceID);
		Microsoft::WRL::ComPtr<ID3D12Resource>& pUploadResource = uploadData.pUploadResource;
		unsigned int bufferWorkRange = uploadData.workRangeInBytes;
	
		copyCommandList->SetResourceState(graphics, pUploadResource.Get(), D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, D3D12_RESOURCE_STATE_COPY_SOURCE);

		copyCommandList->CopyBufferRegion(graphics, pStaticBufferHeap.Get(), bufferStartingOffset, pUploadResource.Get(), 0, bufferWorkRange);

		copyCommandList->SetResourceState(graphics, pUploadResource.Get(), D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);

		uploadData.alreadyUpdated = true; // indicating that resource has been copied and just waits for safe deletion
	}

	if(isStaticBufferUsed)
		copyCommandList->SetResourceState(graphics, pStaticBufferHeap.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
}

void ConstantBufferHeap::UpdateHeap(Graphics& graphics)
{
	THROW_OBJECT_STATE_ERROR_IF("Buffer was not finished", !m_finished);

	unsigned int currFrameIndex = graphics.GetCurrentBufferIndex();

	for (unsigned int updatedResourceDataIndex = m_frequentlyUpdatedResourcesToUpdate.size(); updatedResourceDataIndex > 0; updatedResourceDataIndex--)
	{
		auto& staticResourceToUpdate = m_frequentlyUpdatedResourcesToUpdate.at(updatedResourceDataIndex - 1);

		if (staticResourceToUpdate.updatedAtFrameIndex == currFrameIndex)
		{
			if (staticResourceToUpdate.alreadyUpdated)
			{
				m_frequentlyUpdatedResourcesToUpdate.erase(m_frequentlyUpdatedResourcesToUpdate.begin() + updatedResourceDataIndex - 1);
				continue;
			}
			else
			{
				staticResourceToUpdate.alreadyUpdated = true;
			}
		}

		UpdateResource(graphics, staticResourceToUpdate.bufferIndex, staticResourceToUpdate.data, staticResourceToUpdate.dataSize);
	}
}

void ConstantBufferHeap::UpdateTempResource(Graphics& graphics, unsigned int bufferIndex, void* data, size_t size)
{
	THROW_OBJECT_STATE_ERROR_IF("Buffer was not finished", !m_finished);

	UINT64 bufferStartingOffset = GetOffsetOfTempBufferAtIndex(bufferIndex);
	UINT64 bufferSize = 256;

	UpdateResource(graphics, bufferStartingOffset, bufferSize, data, size);
}

void ConstantBufferHeap::UpdateResource(Graphics& graphics, unsigned int bufferIndex, void* data, size_t size)
{
	THROW_OBJECT_STATE_ERROR_IF("Buffer was not finished", !m_finished);

	UINT64 bufferStartingOffset = GetOffsetOfFrameBufferAtIndex(graphics, bufferIndex);
	UINT64 bufferSize = GetBufferSizeAtIndex(graphics, bufferIndex);

	UpdateResource(graphics, bufferStartingOffset, bufferSize, data, size);
}

void ConstantBufferHeap::UpdateResource(Graphics& graphics, UINT64 bufferStartingOffset, UINT64 bufferSize, void* data, size_t size)
{
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

		THROW_ERROR(pBufferHeap->Map(
			0,
			&readRange,
			&pMappedData
		));

		memcpy_s(static_cast<char*>(pMappedData) + bufferStartingOffset, bufferSize, data, size);

		THROW_INFO_ERROR(pBufferHeap->Unmap(0, &writeRange));
	}
}

void ConstantBufferHeap::UpdateStaticResource(Graphics& graphics, unsigned int bufferIndex, void* data, size_t size)
{
	THROW_OBJECT_STATE_ERROR_IF("Buffer was not finished", !m_finished);

	Microsoft::WRL::ComPtr<ID3D12Resource> pUploadResource;

	// creating upload constant buffer
	{
		constexpr unsigned int bufferAlignment = 256;
		unsigned int alignedSize = std::ceil(float(size) / float(bufferAlignment)) * float(bufferAlignment);

		HRESULT hr;

		D3D12_HEAP_PROPERTIES heapPropeties = {};
		heapPropeties.Type = D3D12_HEAP_TYPE_CUSTOM;
		heapPropeties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;
		heapPropeties.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;
		heapPropeties.VisibleNodeMask = 0;

		D3D12_RESOURCE_DESC resourceDesc = {};
		resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		resourceDesc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
		resourceDesc.Width = alignedSize;
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
			IID_PPV_ARGS(&pUploadResource)
		));
	}

	// update upload constant buffer
	{
		HRESULT hr;

		D3D12_RANGE readRange = {};
		readRange.Begin = 0;
		readRange.End = 0;

		D3D12_RANGE writeRange = {};
		writeRange.Begin = 0;
		writeRange.End = size;

		void* pMappedData = nullptr;

		THROW_ERROR(pUploadResource->Map(
			0,
			&readRange,
			&pMappedData
		));

		memcpy_s(static_cast<char*>(pMappedData), size, data, size);

		THROW_INFO_ERROR(pUploadResource->Unmap(0, &writeRange));
	}

	UploadResource uploadResourceData = {};
	uploadResourceData.pUploadResource = pUploadResource;
	uploadResourceData.staticResourceID = bufferIndex;
	uploadResourceData.workRangeInBytes = size;
	uploadResourceData.updatedAtFrameIndex = graphics.GetCurrentBufferIndex();

	m_uploadResources.push_back(uploadResourceData);
}

void ConstantBufferHeap::UpdateFrequentlyUpdatedStaticResource(Graphics& graphics, unsigned int bufferIndex, void* data, size_t size)
{
	THROW_OBJECT_STATE_ERROR_IF("Buffer was not finished", !m_finished);

	//removing previous update data for currently targeted buffer
	for(size_t updateDataIndex = 0; updateDataIndex < m_frequentlyUpdatedResourcesToUpdate.size(); updateDataIndex++)
		if(m_frequentlyUpdatedResourcesToUpdate.at(updateDataIndex).bufferIndex == bufferIndex)
			m_frequentlyUpdatedResourcesToUpdate.erase(m_frequentlyUpdatedResourcesToUpdate.begin() + updateDataIndex);


	FrequentlyUpdatedResourceData frequentlyUpdatedResourceData = {};
	frequentlyUpdatedResourceData.data = data;
	frequentlyUpdatedResourceData.dataSize = size;
	frequentlyUpdatedResourceData.bufferIndex = bufferIndex;
	frequentlyUpdatedResourceData.updatedAtFrameIndex = graphics.GetCurrentBufferIndex();

	m_frequentlyUpdatedResourcesToUpdate.push_back(frequentlyUpdatedResourceData);
}