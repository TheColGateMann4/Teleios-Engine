#include "ConstantBufferHeap.h"
#include "Macros/ErrorMacros.h"
#include "Graphics.h"
#include "CommandList.h"

#include "Graphics/Resources/GraphicsBuffer.h"

void BufferHeapBase::Initialize(Graphics& graphics)
{
	m_staticHeap.heap = graphics.GetGraphicsBufferAllocatorManager()->RequestBufferAllocator(graphics, 0, 1, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, BufferType::Static);
	m_dynamicHeap.heap = graphics.GetGraphicsBufferAllocatorManager()->RequestBufferAllocator(graphics, 0, 1, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, BufferType::Dynamic);
}

D3D12_GPU_VIRTUAL_ADDRESS BufferHeapBase::GetBufferAddress(TempBufferIndex bufferIndex)
{
	THROW_INTERNAL_ERROR_IF("Tried to access buffer outside of range", bufferIndex.GetIndex() > m_numberOfTempBuffers);

	D3D12_GPU_VIRTUAL_ADDRESS bufferAddress = m_dynamicHeap.heap->GetResource()->GetGPUAddress();
	bufferAddress += m_dynamicHeap.buffers.at(bufferIndex.GetIndex())->byteOffset;

	return bufferAddress;
}

D3D12_GPU_VIRTUAL_ADDRESS BufferHeapBase::GetBufferAddress(Graphics& graphics, DynamicBufferIndex bufferIndex)
{
	D3D12_GPU_VIRTUAL_ADDRESS bufferAddress = m_dynamicHeap.heap->GetResource()->GetGPUAddress();
	bufferAddress += m_dynamicHeap.buffers.at(bufferIndex.GetIndex())->byteOffset;

	return bufferAddress;
}

D3D12_GPU_VIRTUAL_ADDRESS BufferHeapBase::GetBufferAddress(StaticBufferIndex bufferIndex)
{
	D3D12_GPU_VIRTUAL_ADDRESS bufferAddress = m_staticHeap.heap->GetResource()->GetGPUAddress();
	bufferAddress += m_staticHeap.buffers.at(bufferIndex.GetIndex())->byteOffset;

	return bufferAddress;
}

ID3D12Resource* BufferHeapBase::GetDynamicResource() const
{
	return m_dynamicHeap.heap->GetResource()->GetResource();
}

ID3D12Resource* BufferHeapBase::GetStaticResource() const
{
	return m_staticHeap.heap->GetResource()->GetResource();
}

ID3D12Resource* BufferHeapBase::GetTempResource() const
{
	return m_dynamicHeap.heap->GetResource()->GetResource();
}

void BufferHeapBase::UpdateResource(Graphics& graphics, TempBufferIndex bufferIndex, void* data, size_t size)
{
	THROW_INTERNAL_ERROR("Temp buffers currently unsupported");
}

void BufferHeapBase::UpdateResource(Graphics& graphics, DynamicBufferIndex bufferIndex, void* data, size_t size)
{
	BufferAllocatorChunk* chunk = m_dynamicHeap.buffers.at(bufferIndex.GetIndex()).get();

	m_dynamicHeap.heap->Write(graphics, chunk, data, size, 0);
}

void BufferHeapBase::UpdateResource(Graphics& graphics, StaticBufferIndex bufferIndex, void* data, size_t size)
{
	BufferAllocatorChunk* chunk = m_staticHeap.buffers.at(bufferIndex.GetIndex()).get();

	m_staticHeap.heap->Write(graphics, chunk, data, size, 0);
}

void BufferHeapBase::ResizeResource(Graphics& graphics, TempBufferIndex bufferIndex, size_t size)
{
	THROW_INTERNAL_ERROR("Temp buffers currently unsupported");
}

void BufferHeapBase::ResizeResource(Graphics& graphics, DynamicBufferIndex bufferIndex, size_t size)
{
	auto& chunk = m_dynamicHeap.buffers.at(bufferIndex.GetIndex());

	auto reallocated = m_dynamicHeap.heap->Resize(graphics, chunk, size * 3, chunk->stride);

	m_dynamicHeap.buffers.at(bufferIndex.GetIndex()) = std::move(reallocated);
}

void BufferHeapBase::ResizeResource(Graphics& graphics, StaticBufferIndex bufferIndex, size_t size)
{
	std::shared_ptr<BufferAllocatorChunk>& chunk = m_staticHeap.buffers.at(bufferIndex.GetIndex());

	auto reallocated = m_staticHeap.heap->Resize(graphics, chunk, size, chunk->stride);

	m_staticHeap.buffers.at(bufferIndex.GetIndex()) = std::move(reallocated);
}

UINT64 BufferHeapBase::GetOffsetOfBuffer(Graphics& graphics, DynamicBufferIndex bufferIndex)
{
	return GetBufferOffsetAtIndex(m_dynamicHeap, bufferIndex.GetIndex()) + GetSizeOfBuffer(graphics, bufferIndex) * graphics.GetCurrentBufferIndex();
}

UINT64 BufferHeapBase::GetOffsetOfBuffer(TempBufferIndex bufferIndex)
{
	THROW_INTERNAL_ERROR("Temp buffers currently unsupported");

	return 0;
}

UINT64 BufferHeapBase::GetOffsetOfBuffer(StaticBufferIndex bufferIndex)
{
	return GetBufferOffsetAtIndex(m_staticHeap, bufferIndex.GetIndex());
}

UINT64 BufferHeapBase::GetSizeOfBuffer(Graphics& graphics, DynamicBufferIndex bufferIndex)
{
	THROW_INTERNAL_ERROR_IF("Tried to access offset outside constant buffer heap", bufferIndex.GetIndex() > m_dynamicHeap.buffers.size());

	return m_dynamicHeap.buffers.at(bufferIndex.GetIndex())->size;
}

UINT64 BufferHeapBase::GetSizeOfBuffer(Graphics& graphics, StaticBufferIndex bufferIndex)
{
	THROW_INTERNAL_ERROR_IF("Tried to access offset outside constant buffer heap", bufferIndex.GetIndex() > m_staticHeap.buffers.size());

	return m_staticHeap.buffers.at(bufferIndex.GetIndex())->size;
}

UINT64 BufferHeapBase::GetBufferOffsetAtIndex(const HeapData& heapData, unsigned int bufferIndex)
{
	THROW_INTERNAL_ERROR_IF("Tried to access offset outside constant buffer heap", bufferIndex > heapData.buffers.size());

	return heapData.buffers.at(bufferIndex)->byteOffset;
}

TempBufferIndex ConstantBufferHeap::GetNextTempIndex(UINT resourceSize)
{
	THROW_INTERNAL_ERROR("Temp buffers currently unsupported");

	m_numTempBuffersUsed++;

	return TempBufferIndex(m_numTempBuffersUsed - 1);
}

StaticBufferIndex ConstantBufferHeap::RequestMoreStaticSpace(Graphics& graphics, UINT resourceSize)
{
	UINT alignedSize = GetAligned(resourceSize, 256);

	m_staticHeap.buffers.push_back(m_staticHeap.heap->Allocate(graphics, alignedSize, resourceSize));

	return StaticBufferIndex(m_staticHeap.buffers.size() - 1);
}

DynamicBufferIndex ConstantBufferHeap::RequestMoreSpace(Graphics& graphics, UINT resourceSize)
{
	UINT alignedSize = GetAligned(resourceSize, 256);

	m_dynamicHeap.buffers.push_back(m_dynamicHeap.heap->Allocate(graphics, alignedSize * 3, resourceSize));

	return DynamicBufferIndex(m_dynamicHeap.buffers.size() - 1);
}

UINT64 ConstantBufferHeap::GetAligned(UINT64 offset, UINT stride)
{
	return ((offset + 256 - 1) / 256) * 256;
}

void BufferHeap::Initialize(Graphics& graphics)
{
	BufferHeapBase::Initialize(graphics);

	m_numberOfTempBuffers = 0; // we won't use temp buffers in regular buffer heap for now
}

StaticBufferIndex BufferHeap::RequestMoreStaticSpace(Graphics& graphics, UINT resourceSize, UINT stride)
{
	m_staticHeap.buffers.push_back(m_staticHeap.heap->Allocate(graphics, resourceSize, stride));

	return StaticBufferIndex(m_staticHeap.buffers.size() - 1);
}

DynamicBufferIndex BufferHeap::RequestMoreSpace(Graphics& graphics, UINT resourceSize, UINT stride)
{
	m_dynamicHeap.buffers.push_back(m_dynamicHeap.heap->Allocate(graphics, resourceSize * 3, stride));

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

	return bufferData->byteOffset;
}

UINT BufferHeap::GetBufferSize(unsigned int bufferIndex)
{
	THROW_INTERNAL_ERROR_IF("Tried to access buffer outside of range", bufferIndex > m_dynamicHeap.buffers.size());
	
	return m_dynamicHeap.buffers.at(bufferIndex)->size;
}


UINT64 BufferHeap::GetAligned(UINT64 offset, UINT stride)
{
	return ((offset + stride - 1) / stride) * stride;
}