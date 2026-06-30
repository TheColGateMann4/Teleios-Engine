#include "GraphicsBufferSuballocator.h"
#include "Graphics/Core/Graphics.h"

BufferAllocatorChunk::BufferAllocatorChunk(size_t byteOffset_,size_t elementOffset_, size_t size_, GraphicsBufferSuballocator* allocator_)
	:
	byteOffset(byteOffset_),
	elementOffset(elementOffset_),
	size(size_),
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

GraphicsBufferSuballocator::GraphicsBufferSuballocator(Graphics& graphics, unsigned int numElements, unsigned int byteStride)
	:
	m_stride(byteStride)
{
	if (numElements == 0)
		return;

	m_buffer = std::make_unique<GraphicsBuffer>(graphics, numElements, byteStride, GraphicsResource::CPUAccess::notavailable);
}

std::shared_ptr<BufferAllocatorChunk> GraphicsBufferSuballocator::Push(Graphics& graphics, void* data, size_t size, size_t stride)
{
	THROW_INTERNAL_ERROR_IF("Tried to push data with different stride", stride != m_stride)

	auto optPushed = TryPushToFreeBlocks(graphics, size);

	BufferChunkInfo chunk = optPushed ? *optPushed : PushToEnd(size);

	m_usedChunks.push_back(chunk);

	if(m_buffer && m_usedSpace <= m_buffer->GetByteSize())
		m_buffer->Update(graphics, data, size);
	else
	{
		auto uploadBuffer = std::make_unique<GraphicsBuffer>(graphics, size / stride, stride, GraphicsResource::CPUAccess::write);
		uploadBuffer->Update(graphics, data, size);

		m_pendingUploadBuffers.push_back(UploadBufferData(std::move(uploadBuffer), chunk.offset));
	}

	return std::make_shared<BufferAllocatorChunk>(chunk.offset, chunk.offset / stride, chunk.size, this);
}

void GraphicsBufferSuballocator::Free(BufferAllocatorChunk* chunkInfo)
{
	THROW_INTERNAL_ERROR_IF("Passed graphics buffers allocator was NULL", chunkInfo == nullptr);
	THROW_INTERNAL_ERROR_IF("Tried to use chunk from different graphics buffer allocator", chunkInfo->allocator != this);

	auto found = std::find_if(m_usedChunks.begin(), m_usedChunks.end(), 
		[chunkInfo](const BufferChunkInfo& c)
		{
			return c.offset == chunkInfo->byteOffset && c.size == chunkInfo->size;
		}
	);

	THROW_INTERNAL_ERROR_IF("Could not find chunk to free", found == m_usedChunks.end());

	m_freeChunks.push_back(FreedChunkInfo(BufferChunkInfo(chunkInfo->byteOffset, chunkInfo->size)));
	m_usedChunks.erase(found);
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

	if (m_buffer && m_usedSpace <= m_buffer->GetByteSize())
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
		if (uploadBuffer.offset == 2962332 && uploadBuffer.buffer->GetByteSize() == 671424)
			std::cout << "a";

		pipeline.AddBufferRegionToCopyPipeline(DestinationBufferRegionCopyData{ m_buffer.get(), uploadBuffer.offset }, SourceBufferRegionCopyData{ uploadBuffer.buffer.get(), 0, uploadBuffer.buffer->GetByteSize() });

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

std::optional<GraphicsBufferSuballocator::BufferChunkInfo> GraphicsBufferSuballocator::TryPushToFreeBlocks(Graphics& graphics, size_t size)
{
	auto found = GetBestMatchingChunk(graphics, size);

	if (!found)
		return std::nullopt;

	FreedChunkInfo result = **found;

	size_t remainingChunkSpace = result.chunk.size - size;
	if (remainingChunkSpace == 0)
		m_freeChunks.erase(*found);
	else
		(*found)->chunk.size = remainingChunkSpace;

	return result.chunk;
}

GraphicsBufferSuballocator::BufferChunkInfo GraphicsBufferSuballocator::PushToEnd(size_t size)
{
	size_t offset = m_usedSpace;
	m_usedSpace += size;

	return BufferChunkInfo( offset, size);
}

std::optional<std::vector<GraphicsBufferSuballocator::FreedChunkInfo>::iterator> GraphicsBufferSuballocator::GetBestMatchingChunk(Graphics& graphics, size_t size)
{
	std::vector<unsigned int> fenceValues(graphics.GetBufferCount(), 0);

	for (int i = 0; i < graphics.GetBufferCount(); i++)
		fenceValues.at(i) = graphics.GetFence(i)->GetValue();

	auto found = m_freeChunks.end();

	for (auto it = m_freeChunks.begin(); it != m_freeChunks.end(); ++it)
	{
		if (!it->initialized || it->chunk.size < size || fenceValues.at(it->frameIndex) <= it->frameIndex)
			continue;

		if (found == m_freeChunks.end() || it->chunk.size < found->chunk.size)
			found = it;
	}

	if (found != m_freeChunks.end())
		return found;

	return std::nullopt;
}