#pragma once
#include "Includes/CppIncludes.h"
#include "Graphics/Resources/GraphicsBuffer.h"

class BufferAllocatorUpdateListener
{
public:
	virtual ~BufferAllocatorUpdateListener() = default;

	virtual void UpdateCallback() = 0;
};

class GraphicsBufferSuballocator;

class BufferAllocatorChunk
{
public:
	BufferAllocatorChunk(size_t byteOffset_, size_t elementOffset_, size_t size_, unsigned int stride_, GraphicsBufferSuballocator* allocator_);
	~BufferAllocatorChunk();

	BufferAllocatorChunk(BufferAllocatorChunk&& other) noexcept;
	BufferAllocatorChunk& operator=(BufferAllocatorChunk&& other) noexcept;

	BufferAllocatorChunk(const BufferAllocatorChunk& other) = delete;
	BufferAllocatorChunk& operator=(const BufferAllocatorChunk& other) = delete;

public:
	size_t elementOffset;
	size_t byteOffset;
	size_t size;
	unsigned int stride;
	GraphicsBufferSuballocator* allocator;
};

enum class BufferType
{
	Static,
	Dynamic
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
		size_t fenceValue = 0;
		unsigned int frameIndex = 0;
		bool initialized = false;
	};

	struct UploadBufferData
	{
		std::unique_ptr<GraphicsBuffer> buffer;
		size_t dstoffset;
		size_t srcoffset = 0;
	};

public:
	GraphicsBufferSuballocator(Graphics& graphics, unsigned int numElements, unsigned int byteStride, D3D12_RESOURCE_STATES bufferState, BufferType type);

public:
	std::shared_ptr<BufferAllocatorChunk> Allocate(Graphics& graphics, size_t size, unsigned int stride);
	std::shared_ptr<BufferAllocatorChunk> Push(Graphics& graphics, void* data, size_t size, unsigned int stride);
	void Free(BufferAllocatorChunk* chunkInfo);
	void Write(Graphics& graphics, BufferAllocatorChunk* chunkInfo, void* data, size_t size, size_t offset);
	std::shared_ptr<BufferAllocatorChunk> Resize(Graphics& graphics, std::shared_ptr<BufferAllocatorChunk>& chunkInfo, size_t newSize, unsigned int stride);

	// takes all upload buffers and reallocates main data if needed
	void Update(Graphics& graphics);

	GraphicsBuffer* GetResource() const;

	unsigned int GetByteStride() const;

	void RegisterForUpdates(BufferAllocatorUpdateListener* listener);
	void UnregisterFromUpdates(BufferAllocatorUpdateListener* listener);

private:
	std::optional<BufferChunkInfo> TryPushToFreeBlocks(Graphics& graphics, size_t size, unsigned int stride);
	BufferChunkInfo PushToEnd(size_t size, unsigned int stride);

	std::optional<std::vector<FreedChunkInfo>::iterator> GetBestMatchingChunk(Graphics& graphics, size_t size, unsigned int stride);
	void InsertFreeChunk(const BufferChunkInfo& chunkInfo);
	void ShrinkChunk(BufferAllocatorChunk* chunkInfo, size_t newSize);

private:
	std::vector<BufferAllocatorUpdateListener*> m_updateListeners = {};
	std::unique_ptr<GraphicsBuffer> m_buffer;
	std::vector<UploadDataInfo> m_uploads = {};
	std::vector<FreedChunkInfo> m_freeChunks = {};
	std::vector<BufferChunkInfo> m_usedChunks = {};
	std::vector<std::pair<BufferChunkInfo, BufferChunkInfo>> m_selfCopy = {};
	std::vector<UploadBufferData> m_pendingUploadBuffers = {};
	size_t m_usedSpace = 0;
	unsigned int m_stride;
	BufferType m_type;
};