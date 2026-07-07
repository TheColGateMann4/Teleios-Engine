#include "GraphicsBufferSuballocator.h"
#include "Graphics/Core/Graphics.h"

BufferAllocatorChunk::BufferAllocatorChunk(size_t byteOffset_, size_t elementOffset_, size_t size_, unsigned int stride_, GraphicsBufferSuballocator* allocator_)
	:
	byteOffset(byteOffset_),
	elementOffset(elementOffset_),
	size(size_),
	stride(stride_),
	allocator(allocator_)
{

}

BufferAllocatorChunk::~BufferAllocatorChunk()
{
	if(allocator)
		allocator->Free(this);
}

BufferAllocatorChunk::BufferAllocatorChunk(BufferAllocatorChunk&& other) noexcept
	:
	byteOffset(other.byteOffset),
	elementOffset(other.elementOffset),
	size(other.size),
	allocator(other.allocator)
{
	other.byteOffset = 0;
	other.elementOffset = 0;
	other.size = 0;
	other.allocator = nullptr;
}

BufferAllocatorChunk& BufferAllocatorChunk::operator=(BufferAllocatorChunk&& other) noexcept
{
	byteOffset = other.byteOffset;
	elementOffset = other.elementOffset;
	size = other.size;
	allocator = other.allocator;

	other.byteOffset = 0;
	other.elementOffset = 0;
	other.size = 0;
	other.allocator = nullptr;

	return *this;
}

GraphicsBufferSuballocator::GraphicsBufferSuballocator(Graphics& graphics, unsigned int numElements, unsigned int byteStride, D3D12_RESOURCE_STATES bufferState, BufferType type)
	:
	m_stride(byteStride),
	m_type(type)
{
	if (numElements == 0)
		return;

	GraphicsResource::CPUAccess cpuAccess = m_type == BufferType::Static ? GraphicsResource::CPUAccess::notavailable : GraphicsResource::CPUAccess::readwrite;

	m_buffer = std::make_unique<GraphicsBuffer>(graphics, numElements, byteStride, cpuAccess, bufferState);
}

std::shared_ptr<BufferAllocatorChunk> GraphicsBufferSuballocator::Allocate(Graphics& graphics, size_t size, unsigned int stride)
{
	auto optPushed = TryPushToFreeBlocks(graphics, size, stride);

	BufferChunkInfo chunk = optPushed ? *optPushed : PushToEnd(size, stride);

	m_usedChunks.push_back(chunk);

	return std::make_shared<BufferAllocatorChunk>(chunk.offset, chunk.offset / stride, chunk.size, stride, this);
}

std::shared_ptr<BufferAllocatorChunk> GraphicsBufferSuballocator::Push(Graphics& graphics, void* data, size_t size, unsigned int stride)
{
	auto optPushed = TryPushToFreeBlocks(graphics, size, stride);

	BufferChunkInfo chunk = optPushed ? *optPushed : PushToEnd(size, stride);

	m_usedChunks.push_back(chunk);

	if (m_buffer && m_usedSpace <= m_buffer->GetByteSize())
		m_buffer->Update(graphics, data, size, chunk.offset);
	else
	{
		auto uploadBuffer = std::make_unique<GraphicsBuffer>(graphics, size, 1, GraphicsResource::CPUAccess::write);
		uploadBuffer->Update(graphics, data, size);

		m_pendingUploadBuffers.push_back(UploadBufferData(std::move(uploadBuffer), chunk.offset));
	}

	return std::make_shared<BufferAllocatorChunk>(chunk.offset, chunk.offset / stride, chunk.size, stride, this);
}

void GraphicsBufferSuballocator::Free(BufferAllocatorChunk* chunkInfo)
{
	THROW_INTERNAL_ERROR_IF("Passed graphics buffers allocator was NULL", chunkInfo == nullptr);
	THROW_INTERNAL_ERROR_IF("Tried to use chunk from different graphics buffer allocator", chunkInfo->allocator != this);

	// erasing free chunk
	{
		auto found = std::find_if(m_usedChunks.begin(), m_usedChunks.end(),
			[chunkInfo](const BufferChunkInfo& c)
			{
				return c.offset == chunkInfo->byteOffset && c.size == chunkInfo->size;
			}
		);

		THROW_INTERNAL_ERROR_IF("Could not find chunk to free", found == m_usedChunks.end());

		m_usedChunks.erase(found);
	}

	InsertFreeChunk(BufferChunkInfo(chunkInfo->byteOffset, chunkInfo->size));
}

void GraphicsBufferSuballocator::Write(Graphics& graphics, BufferAllocatorChunk* chunkInfo, void* data, size_t size, size_t offset)
{
	THROW_INTERNAL_ERROR_IF("Tried to access memory out of resource bounds", chunkInfo->byteOffset + offset + size > m_buffer->GetByteSize());
	THROW_INTERNAL_ERROR_IF("Tried to access memory out of chunk bounds", size > chunkInfo->size);

	m_buffer->Update(graphics, data, size, chunkInfo->byteOffset + offset);
}

std::shared_ptr<BufferAllocatorChunk> GraphicsBufferSuballocator::Resize(Graphics& graphics, std::shared_ptr<BufferAllocatorChunk>& chunkInfo, size_t newSize, unsigned int stride)
{
	// return the same size
	if (chunkInfo->size == newSize)
		return chunkInfo;

	// shrink
	if (chunkInfo->size > newSize)
	{
		ShrinkChunk(chunkInfo.get(), newSize);
		return chunkInfo;
	}

	// grow
	{
		Free(chunkInfo.get());
		chunkInfo->allocator = nullptr;

		std::shared_ptr<BufferAllocatorChunk> result = Allocate(graphics, newSize, stride);

		if(chunkInfo->byteOffset != result->byteOffset)
		{
			auto& pipeline = graphics.GetRenderer().GetPipeline();

			pipeline.AddBufferRegionToCopyPipeline(
				DestinationBufferRegionCopyData{ m_buffer.get(), result->byteOffset },
				SourceBufferRegionCopyData{ m_buffer.get(), chunkInfo->byteOffset, chunkInfo->size });
		}

		return result;
	}
}

void GraphicsBufferSuballocator::Update(Graphics& graphics)
{
	for (auto& chunk : m_freeChunks)
	{
		if (chunk.initialized)
			continue;

		chunk.frameIndex = graphics.GetCurrentBufferIndex();
		chunk.fenceValue = graphics.GetFence(chunk.frameIndex)->GetValue();
		chunk.initialized = true;
	}

	if (m_buffer && m_usedSpace <= m_buffer->GetByteSize() || m_usedSpace == 0)
		return;
	
	Pipeline& pipeline = graphics.GetRenderer().GetPipeline();

	{
		unsigned int numElements = m_usedSpace / m_stride;
		std::unique_ptr<GraphicsBuffer> newBuffer = std::make_unique<GraphicsBuffer>(graphics, numElements, m_stride, GraphicsResource::CPUAccess::notavailable);

		if (m_buffer)
		{
			pipeline.AddBufferRegionToCopyPipeline(DestinationBufferRegionCopyData{ newBuffer.get(), 0 }, SourceBufferRegionCopyData{ m_buffer.get(), 0, m_buffer->GetByteSize() });

			graphics.GetFrameResourceDeleter()->DeleteResource(graphics, std::move(m_buffer));
		}

		m_buffer = std::move(newBuffer);

		for (auto* updateListener : m_updateListeners)
			updateListener->UpdateCallback();
	}

	for (auto& uploadBuffer : m_pendingUploadBuffers)
	{
		pipeline.AddBufferRegionToCopyPipeline(DestinationBufferRegionCopyData{ m_buffer.get(), uploadBuffer.dstoffset }, SourceBufferRegionCopyData{ uploadBuffer.buffer.get(), 0, uploadBuffer.buffer->GetByteSize() });

		graphics.GetFrameResourceDeleter()->DeleteResource(graphics, std::move(uploadBuffer));
	}

	m_pendingUploadBuffers.clear();
}

GraphicsBuffer* GraphicsBufferSuballocator::GetResource() const
{
	return m_buffer.get();
}

unsigned int GraphicsBufferSuballocator::GetByteStride() const
{
	return m_stride;
}

void GraphicsBufferSuballocator::RegisterForUpdates(BufferAllocatorUpdateListener* listener)
{
	m_updateListeners.push_back(listener);
}

void GraphicsBufferSuballocator::UnregisterFromUpdates(BufferAllocatorUpdateListener* listener)
{
	std::erase(m_updateListeners, listener);
}


UINT64 GetAligned(UINT64 value, UINT stride)
{
	return ((value + stride - 1) / stride) * stride;
}

std::optional<GraphicsBufferSuballocator::BufferChunkInfo> GraphicsBufferSuballocator::TryPushToFreeBlocks(Graphics& graphics, size_t size, unsigned int stride)
{
	auto found = GetBestMatchingChunk(graphics, size, stride);

	if (!found)
		return std::nullopt;

	auto& foundChunk = (*found)->chunk;

	BufferChunkInfo result = foundChunk;
	result.offset = GetAligned(result.offset, stride);
	result.size = size;

	BufferChunkInfo remainingBefore = foundChunk;
	remainingBefore.size = result.offset - remainingBefore.offset;

	BufferChunkInfo remainingAfter = foundChunk;
	remainingAfter.offset = result.offset + size;
	remainingAfter.size -= remainingBefore.size + size;

	m_freeChunks.erase(*found);

	if (result.offset != remainingBefore.offset)
		InsertFreeChunk(remainingBefore);

	if (remainingAfter.size != 0)
		InsertFreeChunk(remainingAfter);

	return result;
}

GraphicsBufferSuballocator::BufferChunkInfo GraphicsBufferSuballocator::PushToEnd(size_t size, unsigned int stride)
{
	size_t offset = GetAligned(m_usedSpace, stride);

	if (offset != m_usedSpace)
		m_freeChunks.push_back(FreedChunkInfo(BufferChunkInfo(m_usedSpace, offset - m_usedSpace)));

	m_usedSpace = offset + size;
	return BufferChunkInfo( offset, size);
}

std::optional<std::vector<GraphicsBufferSuballocator::FreedChunkInfo>::iterator> GraphicsBufferSuballocator::GetBestMatchingChunk(Graphics& graphics, size_t size, unsigned int stride)
{
	std::vector<unsigned int> fenceValues(graphics.GetBufferCount(), 0);

	for (int i = 0; i < graphics.GetBufferCount(); i++)
		fenceValues.at(i) = graphics.GetFence(i)->GetValue();

	auto found = m_freeChunks.end();

	for (auto it = m_freeChunks.begin(); it != m_freeChunks.end(); ++it)
	{
		size_t alignedOffset = GetAligned(it->chunk.offset, stride);

		if (alignedOffset - it->chunk.offset >= it->chunk.size)
			continue;

		size_t foundSize = it->chunk.size - (alignedOffset - it->chunk.offset);

		if (!it->initialized || foundSize < size || fenceValues.at(it->frameIndex) <= it->fenceValue)
			continue;

		if (found == m_freeChunks.end() || foundSize < found->chunk.size)
			found = it;
	}

	if (found != m_freeChunks.end())
		return found;

	return std::nullopt;
}

void GraphicsBufferSuballocator::InsertFreeChunk(const BufferChunkInfo& chunkInfo)
{
	auto chunk = FreedChunkInfo(chunkInfo);

	auto it = std::lower_bound(m_freeChunks.begin(), m_freeChunks.end(), chunk,
		[](const FreedChunkInfo& a, const FreedChunkInfo& b)
		{
			return a.chunk.offset < b.chunk.offset;
		});

	// try merge with next entry
	if (it != m_freeChunks.end())
	{
		if (chunk.chunk.offset + chunk.chunk.size == it->chunk.offset)
		{
			chunk.chunk.size += it->chunk.size;
			it = m_freeChunks.erase(it);
		}
	}

	// try merge with previous one
	if (it != m_freeChunks.begin() &&
		std::prev(it)->chunk.offset + std::prev(it)->chunk.size == chunk.chunk.offset)
	{
		std::prev(it)->chunk.size += chunk.chunk.size;
	}
	else
	{
		m_freeChunks.insert(it, chunk);
	}
}

void GraphicsBufferSuballocator::ShrinkChunk(BufferAllocatorChunk* chunkInfo, size_t newSize)
{
	auto found = std::find_if(m_usedChunks.begin(), m_usedChunks.end(),
		[chunkInfo](const BufferChunkInfo& c)
		{
			return c.offset == chunkInfo->byteOffset && c.size == chunkInfo->size;
		}
	);

	THROW_INTERNAL_ERROR_IF("Could not find chunk to free", found == m_usedChunks.end());

	found->size = newSize;

	InsertFreeChunk(BufferChunkInfo(chunkInfo->byteOffset + newSize, chunkInfo->size - newSize));

	chunkInfo->size = newSize;
}