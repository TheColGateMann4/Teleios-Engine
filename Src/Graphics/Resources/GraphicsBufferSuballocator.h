#pragma once
#include "Includes/CppIncludes.h"
#include "Graphics/Resources/GraphicsBuffer.h"

class GraphicsBufferSuballocator;

class BufferAllocatorChunk
{
public:
	BufferAllocatorChunk(size_t offset_, size_t size_, GraphicsBufferSuballocator* allocator_);
	~BufferAllocatorChunk();

public:
	size_t offset;
	size_t size;
	GraphicsBufferSuballocator* allocator;
};

class GraphicsBufferSuballocator
{
private:
	struct BufferChunkInfo
	{
		size_t offset;
		size_t size;
	};

	struct UploadDataInfo
	{
		BufferChunkInfo target;
		std::unique_ptr<GraphicsBuffer> uploadBuffer;
	};

	struct FreedChunkInfo
	{
		BufferChunkInfo chunk;
		size_t fenceValue;
		unsigned int frameIndex;
		bool initialized = false;
	};

	struct UploadBufferData
	{
		std::unique_ptr<GraphicsBuffer> buffer;
		size_t offset;
	};

public:
	GraphicsBufferSuballocator(Graphics& graphics, unsigned int numElements, unsigned int byteStride);

public:
	std::shared_ptr<BufferAllocatorChunk> Push(Graphics& graphics, void* data, size_t size);
	void Free(BufferAllocatorChunk* chunkInfo);

	// takes all upload buffers and reallocates main data if needed
	void Update(Graphics& graphics);

	GraphicsBuffer* GetResource() const;

	unsigned int GetByteStride() const;

private:
	std::optional<BufferChunkInfo> TryPushToFreeBlocks(Graphics& graphics, size_t size);
	BufferChunkInfo PushToEnd(size_t size);

	std::optional<std::vector<FreedChunkInfo>::iterator> GetBestMatchingChunk(Graphics& graphics, size_t size);

private:
	std::unique_ptr<GraphicsBuffer> m_buffer;
	std::vector<UploadDataInfo> m_uploads = {};
	std::vector<FreedChunkInfo> m_freeChunks = {};
	std::vector<BufferChunkInfo> m_usedChunks = {};
	std::vector<UploadBufferData> m_pendingUploadBuffers = {};
	size_t m_usedSpace = 0;
	unsigned int m_stride;
};