#include "ConstantBufferHeap.h"
#include "Macros/ErrorMacros.h"
#include "Graphics.h"
#include "CommandList.h"

#include "Graphics/Resources/GraphicsBuffer.h"

BufferHeapBase::~BufferHeapBase()
{
	if(pBufferHeapMappedData && m_dynamicHeap.heap)
		m_dynamicHeap.heap->UnMap();
}

void BufferHeapBase::Finish(Graphics& graphics)
{
	THROW_OBJECT_STATE_ERROR_IF("Tried to finish when constant buffer heap is finished", m_finished);

	if (m_dynamicHeap.combinedSize == 0)
		m_dynamicHeap.combinedSize = 1024 * 8;

	if (m_staticHeap.combinedSize == 0)
		m_staticHeap.combinedSize = 1024 * 8;

	m_dynamicHeap.heap = std::make_unique<GraphicsBuffer>(graphics, m_dynamicHeap.combinedSize + m_numberOfTempBuffers, 1, GraphicsResource::CPUAccess::readwrite);

	pBufferHeapMappedData = m_dynamicHeap.heap->Map(graphics);

	m_staticHeap.heap = std::make_unique<GraphicsBuffer>(graphics, m_staticHeap.combinedSize, 1, GraphicsResource::CPUAccess::notavailable);

	m_finished = true;
}

UINT64 BufferHeapBase::GetOffsetOfBuffer(Graphics& graphics, DynamicBufferIndex bufferIndex)
{
	return GetBufferOffsetAtIndex(m_dynamicHeap, bufferIndex.GetIndex()) + GetSizeOfBuffer(graphics, bufferIndex) * graphics.GetCurrentBufferIndex();
}

UINT64 BufferHeapBase::GetOffsetOfBuffer(TempBufferIndex bufferIndex)
{
	return m_dynamicHeap.combinedSize + bufferIndex.GetIndex() * 256;
}

UINT64 BufferHeapBase::GetOffsetOfBuffer(StaticBufferIndex bufferIndex)
{
	return GetBufferOffsetAtIndex(m_staticHeap, bufferIndex.GetIndex());
}

UINT64 BufferHeapBase::GetSizeOfBuffer(Graphics& graphics, DynamicBufferIndex bufferIndex)
{
	THROW_INTERNAL_ERROR_IF("Tried to access offset outside constant buffer heap", bufferIndex.GetIndex() > m_dynamicHeap.buffers.size());

	return m_dynamicHeap.buffers.at(bufferIndex.GetIndex()).size;
}

UINT64 BufferHeapBase::GetSizeOfBuffer(Graphics& graphics, StaticBufferIndex bufferIndex)
{
	THROW_INTERNAL_ERROR_IF("Tried to access offset outside constant buffer heap", bufferIndex.GetIndex() > m_staticHeap.buffers.size());

	return m_staticHeap.buffers.at(bufferIndex.GetIndex()).size;
}

UINT64 BufferHeapBase::GetBufferOffsetAtIndex(const HeapData& heapData, unsigned int bufferIndex)
{
	THROW_INTERNAL_ERROR_IF("Tried to access offset outside constant buffer heap", bufferIndex > heapData.buffers.size());

	return heapData.buffers.at(bufferIndex).offset;
}

D3D12_GPU_VIRTUAL_ADDRESS BufferHeapBase::GetBufferAddress(TempBufferIndex bufferIndex)
{
	THROW_INTERNAL_ERROR_IF("Tried to access buffer outside of range", bufferIndex.GetIndex() > m_numberOfTempBuffers);

	D3D12_GPU_VIRTUAL_ADDRESS bufferAddress = m_dynamicHeap.heap->GetGPUAddress();
	bufferAddress += GetOffsetOfBuffer(bufferIndex);

	return bufferAddress;
}

D3D12_GPU_VIRTUAL_ADDRESS BufferHeapBase::GetBufferAddress(Graphics& graphics, DynamicBufferIndex bufferIndex)
{
	D3D12_GPU_VIRTUAL_ADDRESS bufferAddress = m_dynamicHeap.heap->GetGPUAddress();
	bufferAddress += GetOffsetOfBuffer(graphics, bufferIndex);

	return bufferAddress;
}

D3D12_GPU_VIRTUAL_ADDRESS BufferHeapBase::GetBufferAddress(StaticBufferIndex bufferIndex)
{
	D3D12_GPU_VIRTUAL_ADDRESS bufferAddress = m_staticHeap.heap->GetGPUAddress();
	bufferAddress += GetOffsetOfBuffer(bufferIndex);

	return bufferAddress;
}

void BufferHeapBase::CopyResources(Graphics& graphics, CommandList* copyCommandList)
{
	THROW_OBJECT_STATE_ERROR_IF("Buffer was not finished", !m_finished);

	if (m_uploadResources.empty())
		return;

	copyCommandList->SetResourceState(graphics, m_staticHeap.heap.get(), D3D12_RESOURCE_STATE_COPY_DEST);

	for (auto& uploadData : m_uploadResources)
	{
		std::unique_ptr<GraphicsBuffer>& pUploadResource = uploadData.uploadResource;
		unsigned int bufferWorkRange = uploadData.workRangeInBytes;
		UINT64 bufferStartingOffset = GetOffsetOfBuffer(uploadData.staticResourceID);
	
		pUploadResource->CopyPartiallyTo(graphics, copyCommandList, 0, bufferWorkRange, m_staticHeap.heap.get(), bufferStartingOffset);

		graphics.GetFrameResourceDeleter()->DeleteResource(graphics, std::move(pUploadResource));
	}

	copyCommandList->SetResourceState(graphics, m_staticHeap.heap.get(), D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);

	m_uploadResources.clear();
}

ID3D12Resource* BufferHeapBase::GetDynamicResource() const
{
	return m_dynamicHeap.heap->GetResource();
}

ID3D12Resource* BufferHeapBase::GetStaticResource() const
{
	return m_staticHeap.heap->GetResource();
}

ID3D12Resource* BufferHeapBase::GetTempResource() const
{
	return m_dynamicHeap.heap->GetResource();
}

void BufferHeapBase::UpdateHeap(Graphics& graphics)
{
	THROW_OBJECT_STATE_ERROR_IF("Buffer was not finished", !m_finished);

	unsigned int currFrameIndex = graphics.GetCurrentBufferIndex();

	for (auto& resourceToUpdate : m_frequentlyUpdatedResourcesToUpdate)
	{
		if (resourceToUpdate.second.updated)
			continue;

		UpdateResource(graphics, DynamicBufferIndex(resourceToUpdate.first), resourceToUpdate.second.data, resourceToUpdate.second.dataSize);
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

void BufferHeapBase::UpdateResource(Graphics& graphics, TempBufferIndex bufferIndex, void* data, size_t size)
{
	THROW_OBJECT_STATE_ERROR_IF("Buffer was not finished", !m_finished);

	UINT64 bufferStartingOffset = GetOffsetOfBuffer(bufferIndex);
	UINT64 bufferSize = 256;

	UpdateResource(bufferStartingOffset, bufferSize, data, size);
}

void BufferHeapBase::UpdateResource(Graphics& graphics, DynamicBufferIndex bufferIndex, void* data, size_t size)
{
	THROW_OBJECT_STATE_ERROR_IF("Buffer was not finished", !m_finished);

	UINT64 bufferStartingOffset = GetOffsetOfBuffer(graphics, bufferIndex);
	UINT64 bufferSize = GetSizeOfBuffer(graphics, bufferIndex);

	UpdateResource(bufferStartingOffset, bufferSize, data, size);
}

void BufferHeapBase::UpdateResource(UINT64 bufferStartingOffset, UINT64 bufferSize, void* data, size_t size)
{
	THROW_INTERNAL_ERROR_IF("Tried to update buffer as larger than it is", size > bufferSize);
	THROW_INTERNAL_ERROR_IF("Passed sizes were larger than buffer itself", size > m_dynamicHeap.combinedSize || bufferSize > m_dynamicHeap.combinedSize);

	void* dest = static_cast<char*>(pBufferHeapMappedData) + bufferStartingOffset;

	memcpy_s(dest, bufferSize, data, size);
}

void BufferHeapBase::UpdateResource(Graphics& graphics, StaticBufferIndex bufferIndex, void* data, size_t size)
{
	THROW_OBJECT_STATE_ERROR_IF("Buffer was not finished", !m_finished);

	std::unique_ptr<GraphicsBuffer> uploadResource = std::make_unique<GraphicsBuffer>(graphics, size, 1, GraphicsResource::CPUAccess::write);;

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

void BufferHeapBase::UpdateFrequentlyUpdatedResource(Graphics& graphics, DynamicBufferIndex bufferIndex, void* data, size_t size)
{
	THROW_OBJECT_STATE_ERROR_IF("Buffer was not finished", !m_finished);

	FrequentlyUpdatedResourceData frequentlyUpdatedResourceData = {};
	frequentlyUpdatedResourceData.data = data;
	frequentlyUpdatedResourceData.dataSize = size;
	frequentlyUpdatedResourceData.frameIndex = graphics.GetCurrentBufferIndex();

	m_frequentlyUpdatedResourcesToUpdate[bufferIndex.GetIndex()] = frequentlyUpdatedResourceData;
}

TempBufferIndex ConstantBufferHeap::GetNextTempIndex(UINT resourceSize)
{
	THROW_INTERNAL_ERROR_IF("Temp buffer size was higher than 256", resourceSize > 256);
	THROW_INTERNAL_ERROR_IF("Ran out of space for temp buffers", m_numTempBuffersUsed == m_numberOfTempBuffers);

	m_numTempBuffersUsed++;

	return TempBufferIndex(m_numTempBuffersUsed - 1);
}

StaticBufferIndex ConstantBufferHeap::RequestMoreStaticSpace(UINT resourceSize)
{
	THROW_OBJECT_STATE_ERROR_IF("Tried to Request more space when constant buffer heap is finished", m_finished);

	UINT alignedSize = GetAligned(resourceSize, 256);

	BufferData bufferData =
	{
		.offset = m_staticHeap.combinedSize,
		.size = resourceSize,
		.stride = 256
	};

	m_staticHeap.buffers.push_back(bufferData);
	m_staticHeap.combinedSize += alignedSize;

	return StaticBufferIndex(m_staticHeap.buffers.size() - 1);
}

DynamicBufferIndex ConstantBufferHeap::RequestMoreSpace(Graphics& graphics, UINT resourceSize)
{
	THROW_OBJECT_STATE_ERROR_IF("Tried to Request more space when constant buffer heap is finished", m_finished);

	UINT alignedSize = GetAligned(resourceSize, 256);

	BufferData bufferData =
	{
		.offset = m_dynamicHeap.combinedSize,
		.size = resourceSize,
		.stride = 256
	};

	m_dynamicHeap.buffers.push_back(bufferData);
	m_dynamicHeap.combinedSize += alignedSize * graphics.GetBufferCount();

	return DynamicBufferIndex(m_dynamicHeap.buffers.size() - 1);
}

UINT64 ConstantBufferHeap::GetAligned(UINT64 offset, UINT stride)
{
	return ((offset + 256 - 1) / 256) * 256;
}

BufferHeap::BufferHeap()
{
	m_numberOfTempBuffers = 0; // we won't use temp buffers in regular buffer heap for now
}

StaticBufferIndex BufferHeap::RequestMoreStaticSpace(UINT resourceSize, UINT stride)
{
	THROW_OBJECT_STATE_ERROR_IF("Tried to Request more space when constant buffer heap is finished", m_finished);

	UINT64 offset = GetAligned(m_dynamicHeap.combinedSize, stride);

	BufferData bufferData =
	{
		.offset = m_staticHeap.combinedSize,
		.size = resourceSize,
		.stride = stride
	};

	m_staticHeap.buffers.push_back(bufferData);

	m_staticHeap.combinedSize = offset + resourceSize;

	return StaticBufferIndex(m_staticHeap.buffers.size() - 1);
}

DynamicBufferIndex BufferHeap::RequestMoreSpace(Graphics& graphics, UINT resourceSize, UINT stride)
{
	THROW_OBJECT_STATE_ERROR_IF("Tried to Request more space when constant buffer heap is finished", m_finished);

	UINT64 offset = GetAligned(m_dynamicHeap.combinedSize, stride);

	BufferData bufferData =
	{
		.offset = m_dynamicHeap.combinedSize,
		.size = resourceSize,
		.stride = stride
	};

	m_dynamicHeap.buffers.push_back(bufferData);
	m_dynamicHeap.combinedSize = offset;
	m_dynamicHeap.combinedSize += resourceSize * graphics.GetBufferCount();

	return DynamicBufferIndex(m_dynamicHeap.buffers.size() - 1);
}

UINT64 BufferHeap::GetOffsetOfBuffer(Graphics& graphics, unsigned int bufferIndex)
{
	return GetOffsetOfBuffer(graphics.GetCurrentBufferIndex(), bufferIndex);
}

UINT64 BufferHeap::GetOffsetOfBuffer(unsigned int frameIndex, unsigned int bufferIndex)
{
	THROW_INTERNAL_ERROR_IF("Tried to access buffer outside of range", bufferIndex > m_dynamicHeap.buffers.size());

	return GetBufferOffset(bufferIndex) + GetBufferSize(bufferIndex) * frameIndex;
}

UINT64 BufferHeap::GetBufferOffset(unsigned int bufferIndex)
{
	auto bufferData = m_dynamicHeap.buffers.at(bufferIndex);

	return bufferData.offset / bufferData.size;
}

UINT BufferHeap::GetBufferSize(unsigned int bufferIndex)
{
	THROW_INTERNAL_ERROR_IF("Tried to access buffer outside of range", bufferIndex > m_dynamicHeap.buffers.size());
	
	UINT64 bufferOffset = m_dynamicHeap.buffers.at(bufferIndex).size;

	if (bufferIndex == m_dynamicHeap.buffers.size() - 1)
		return m_dynamicHeap.combinedSize  - bufferOffset;
	
	UINT64 nextBufferOffset = m_dynamicHeap.buffers.at(bufferIndex).size;
	return nextBufferOffset - bufferOffset;
}


UINT64 BufferHeap::GetAligned(UINT64 offset, UINT stride)
{
	return ((offset + stride - 1) / stride) * stride;
}