#include "ConstantBufferHeap.h"
#include "Macros/ErrorMacros.h"
#include "Graphics.h"
#include "CommandList.h"

#include "Graphics/Resources/GraphicsBuffer.h"

ConstantBufferHeap::~ConstantBufferHeap()
{
	if(pBufferHeapMappedData && m_bufferHeap)
		m_bufferHeap->UnMap();
}

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

	THROW_INTERNAL_ERROR_IF("Combined static resource size wasn't multiple of 256", m_combinedSize % 256 != 0);
	m_bufferHeap = std::make_unique<GraphicsBuffer>(graphics, m_combinedSize / 256 + m_numberOfTempBuffers, 256, GraphicsResource::CPUAccess::readwrite);

	pBufferHeapMappedData = m_bufferHeap->Map(graphics);

	THROW_INTERNAL_ERROR_IF("Combined static resource size wasn't multiple of 256", m_combinedSizeStaticBuffer % 256 != 0);
	m_staticBufferHeap = std::make_unique<GraphicsBuffer>(graphics, m_combinedSizeStaticBuffer / 256, 256, GraphicsResource::CPUAccess::notavailable);

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

	D3D12_GPU_VIRTUAL_ADDRESS bufferAddress = m_bufferHeap->GetGPUAddress();
	bufferAddress += GetOffsetOfTempBufferAtIndex(bufferIndex);

	return bufferAddress;
}

D3D12_GPU_VIRTUAL_ADDRESS ConstantBufferHeap::GetBufferAddress(Graphics& graphics, unsigned int bufferIndex)
{
	D3D12_GPU_VIRTUAL_ADDRESS bufferAddress = m_bufferHeap->GetGPUAddress();
	bufferAddress += GetOffsetOfFrameBufferAtIndex(graphics, bufferIndex);

	return bufferAddress;
}

D3D12_GPU_VIRTUAL_ADDRESS ConstantBufferHeap::GetStaticBufferAddress(unsigned int bufferIndex)
{
	D3D12_GPU_VIRTUAL_ADDRESS bufferAddress = m_staticBufferHeap->GetGPUAddress();
	bufferAddress += GetOffsetOfStaticBufferAtIndex(bufferIndex);

	return bufferAddress;
}

void ConstantBufferHeap::CopyResources(Graphics& graphics, CommandList* copyCommandList)
{
	THROW_OBJECT_STATE_ERROR_IF("Buffer was not finished", !m_finished);

	if (m_uploadResources.empty())
		return;

	copyCommandList->SetResourceState(graphics, m_staticBufferHeap.get(), D3D12_RESOURCE_STATE_COPY_DEST);

	for (auto& uploadData : m_uploadResources)
	{
		std::unique_ptr<GraphicsBuffer>& pUploadResource = uploadData.uploadResource;
		unsigned int bufferWorkRange = uploadData.workRangeInBytes;
		UINT64 bufferStartingOffset = GetOffsetOfStaticBufferAtIndex(uploadData.staticResourceID);
	
		pUploadResource->CopyPartiallyTo(graphics, copyCommandList, 0, bufferWorkRange, m_staticBufferHeap.get(), bufferStartingOffset);

		graphics.GetFrameResourceDeleter()->DeleteResource(graphics, std::move(pUploadResource));
	}

	copyCommandList->SetResourceState(graphics, m_staticBufferHeap.get(), D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);

	m_uploadResources.clear();
}

void ConstantBufferHeap::UpdateHeap(Graphics& graphics)
{
	THROW_OBJECT_STATE_ERROR_IF("Buffer was not finished", !m_finished);

	unsigned int currFrameIndex = graphics.GetCurrentBufferIndex();

	for (auto& resourceToUpdate : m_frequentlyUpdatedResourcesToUpdate)
	{
		if (resourceToUpdate.second.updated)
			continue;

		UpdateResource(graphics, resourceToUpdate.first, resourceToUpdate.second.data, resourceToUpdate.second.dataSize);
		resourceToUpdate.second.updated = true;
	}

	std::erase_if(m_frequentlyUpdatedResourcesToUpdate, 
		[&graphics](const auto& e)
		{
			const auto& v = e.second;
			return v.updated == true && v.frameIndex == graphics.GetCurrentBufferIndex();
		}
	);
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
	THROW_INTERNAL_ERROR_IF("Passed sizes were larger than buffer itself", size > m_combinedSize || bufferSize > m_combinedSize);

	void* dest = static_cast<char*>(pBufferHeapMappedData) + bufferStartingOffset;

	memcpy_s(dest, bufferSize, data, size);
}

void ConstantBufferHeap::UpdateStaticResource(Graphics& graphics, unsigned int bufferIndex, void* data, size_t size)
{
	THROW_OBJECT_STATE_ERROR_IF("Buffer was not finished", !m_finished);

	constexpr unsigned int bufferAlignment = 256;
	unsigned int alignedSize = std::ceil(float(size) / float(bufferAlignment)) * float(bufferAlignment);

	std::unique_ptr<GraphicsBuffer> uploadResource = std::make_unique<GraphicsBuffer>(graphics, alignedSize / 256, 256, GraphicsResource::CPUAccess::write);;

	// update upload constant buffer
	{
		void* pMappedData = nullptr;

		pMappedData = uploadResource->Map(graphics);

		memcpy_s(static_cast<char*>(pMappedData), size, data, size);

		uploadResource->UnMap(0, size);
	}

	UploadResource uploadResourceData = {};
	uploadResourceData.uploadResource = std::move(uploadResource);
	uploadResourceData.staticResourceID = bufferIndex;
	uploadResourceData.workRangeInBytes = size;

	m_uploadResources.push_back(std::move(uploadResourceData));
}

void ConstantBufferHeap::UpdateFrequentlyUpdatedStaticResource(Graphics& graphics, unsigned int bufferIndex, void* data, size_t size)
{
	THROW_OBJECT_STATE_ERROR_IF("Buffer was not finished", !m_finished);

	FrequentlyUpdatedResourceData frequentlyUpdatedResourceData = {};
	frequentlyUpdatedResourceData.data = data;
	frequentlyUpdatedResourceData.dataSize = size;
	frequentlyUpdatedResourceData.frameIndex = graphics.GetCurrentBufferIndex();

	m_frequentlyUpdatedResourcesToUpdate[bufferIndex] = frequentlyUpdatedResourceData;
}