#pragma once
#include "Includes/CppIncludes.h"
#include "Includes/DirectXIncludes.h"
#include "Includes/WRLNoWarnings.h"
#include "Macros/ErrorMacros.h"
#include "Graphics/Resources/GraphicsBufferSuballocator.h"

class Graphics;
class CommandList;
class GraphicsBuffer;

template<class Tag>
class BufferIndex
{
	friend class BufferHeapBase;
	friend class ConstantBufferHeap;
	friend class BufferHeap;
public:
	explicit BufferIndex()
		:
		index(0),
		initialized(false)
	{}
protected:
	explicit BufferIndex(unsigned int index_)
		:
		index(index_),
		initialized(true)
	{}

public:
	unsigned int GetIndex() const
	{
		THROW_INTERNAL_ERROR_IF("Tried to use invalid buffer index", !initialized);

		return index;
	}

private:
	unsigned int index;
	bool initialized;
};

struct DynamicBufferTag {};
struct StaticBufferTag {};
struct TempBufferTag {};

// Stronly typed index varibles for clean interface
using DynamicBufferIndex = BufferIndex<DynamicBufferTag>;
using StaticBufferIndex = BufferIndex<StaticBufferTag>;
using TempBufferIndex = BufferIndex<TempBufferTag>;

class BufferHeapBase
{
protected:
	struct HeapData
	{
		std::shared_ptr<GraphicsBufferSuballocator> heap;
		std::vector<std::shared_ptr<BufferAllocatorChunk>> buffers = {};
	};

public:
	virtual ~BufferHeapBase() = default;

	void Initialize(Graphics& graphics);

public:	// At runtime
	D3D12_GPU_VIRTUAL_ADDRESS GetBufferAddress(TempBufferIndex bufferIndex);
	D3D12_GPU_VIRTUAL_ADDRESS GetBufferAddress(Graphics& graphics, DynamicBufferIndex bufferIndex);
	D3D12_GPU_VIRTUAL_ADDRESS GetBufferAddress(StaticBufferIndex bufferIndex);

	ID3D12Resource* GetDynamicResource() const;
	ID3D12Resource* GetStaticResource() const;
	ID3D12Resource* GetTempResource() const;

	void UpdateResource(Graphics& graphics, TempBufferIndex bufferIndex, void* data, size_t size);
	void UpdateResource(Graphics& graphics, DynamicBufferIndex bufferIndex, void* data, size_t size);
	void UpdateResource(Graphics& graphics, StaticBufferIndex bufferIndex, void* data, size_t size);

	void ResizeResource(Graphics& graphics, TempBufferIndex bufferIndex, size_t size);
	void ResizeResource(Graphics& graphics, DynamicBufferIndex bufferIndex, size_t size);
	void ResizeResource(Graphics& graphics, StaticBufferIndex bufferIndex, size_t size);

private:
	UINT64 GetOffsetOfBuffer(Graphics& graphics, DynamicBufferIndex bufferIndex);
	UINT64 GetOffsetOfBuffer(TempBufferIndex bufferIndex);
	UINT64 GetOffsetOfBuffer(StaticBufferIndex bufferIndex);

	UINT64 GetSizeOfBuffer(Graphics& graphics, DynamicBufferIndex bufferIndex);
	UINT64 GetSizeOfBuffer(Graphics& graphics, StaticBufferIndex bufferIndex);

	UINT64 GetBufferOffsetAtIndex(const HeapData& heapData, unsigned int bufferIndex);

protected:
	// static resources - on GPU-only memory
	HeapData m_staticHeap;
	
	// non static resources - on shared memory; size multiplied by n frames-in-flight
	HeapData m_dynamicHeap;

	// temp resources
	// each buffer is 256 bytes
	UINT64 m_numberOfTempBuffers = 1024;
	UINT64 m_numTempBuffersUsed = 0;
};

class ConstantBufferHeap : public BufferHeapBase
{
public:  // At program initialization
	TempBufferIndex GetNextTempIndex(UINT resourceSize);
	StaticBufferIndex RequestMoreStaticSpace(Graphics& graphics, UINT resourceSize);
	DynamicBufferIndex RequestMoreSpace(Graphics& graphics, UINT resourceSize);

protected:
	UINT64 GetAligned(UINT64 offset, UINT stride);
};

class BufferHeap : public BufferHeapBase
{
public:
	void Initialize(Graphics& graphics);

public:  // At program initialization
	StaticBufferIndex RequestMoreStaticSpace(Graphics& graphics, UINT resourceSize, UINT stride);
	DynamicBufferIndex RequestMoreSpace(Graphics& graphics, UINT resourceSize, UINT stride);

public: // At runtime
	// returns index offset of element in buffer
	UINT64 GetOffsetOfBuffer(Graphics& graphics, unsigned int bufferIndex);
	UINT64 GetOffsetOfBuffer(unsigned int frameIndex, unsigned int bufferIndex);

private:
	UINT64 GetBufferOffset(unsigned int bufferIndex);
	UINT GetBufferSize(unsigned int bufferIndex);

protected:
	UINT64 GetAligned(UINT64 offset, UINT stride);
};