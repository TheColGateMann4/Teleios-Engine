#pragma once
#include "Includes/CppIncludes.h"
#include "Includes/DirectXIncludes.h"
#include "Includes/WRLNoWarnings.h"
#include "Macros/ErrorMacros.h"

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
	struct BufferData
	{
		UINT64 offset;
		UINT size;
		UINT stride;
	};

	struct HeapData
	{
		std::unique_ptr<GraphicsBuffer> heap;
		std::vector<BufferData> buffers = {};
		UINT64 combinedSize = 0;
	};

protected:
	BufferHeapBase() = default;
	virtual ~BufferHeapBase();

public:  // At program initialization
	void Finish(Graphics& graphics);

public:	// At runtime
	void CopyResources(Graphics& graphics, CommandList* copyCommandList);

	D3D12_GPU_VIRTUAL_ADDRESS GetBufferAddress(TempBufferIndex bufferIndex);
	D3D12_GPU_VIRTUAL_ADDRESS GetBufferAddress(Graphics& graphics, DynamicBufferIndex bufferIndex);
	D3D12_GPU_VIRTUAL_ADDRESS GetBufferAddress(StaticBufferIndex bufferIndex);

	ID3D12Resource* GetDynamicResource() const;
	ID3D12Resource* GetStaticResource() const;
	ID3D12Resource* GetTempResource() const;

	void UpdateHeap(Graphics& graphics);

	void UpdateResource(Graphics& graphics, TempBufferIndex bufferIndex, void* data, size_t size);
	void UpdateResource(Graphics& graphics, DynamicBufferIndex bufferIndex, void* data, size_t size);
	void UpdateResource(Graphics& graphics, StaticBufferIndex bufferIndex, void* data, size_t size);
	void UpdateFrequentlyUpdatedResource(Graphics& graphics, DynamicBufferIndex bufferIndex, void* data, size_t size);

private:
	void UpdateResource(UINT64 bufferStartingOffset, UINT64 bufferSize, void* data, size_t size);

private:
	UINT64 GetOffsetOfBuffer(Graphics& graphics, DynamicBufferIndex bufferIndex);
	UINT64 GetOffsetOfBuffer(TempBufferIndex bufferIndex);
	UINT64 GetOffsetOfBuffer(StaticBufferIndex bufferIndex);

	UINT64 GetSizeOfBuffer(Graphics& graphics, DynamicBufferIndex bufferIndex);
	UINT64 GetSizeOfBuffer(Graphics& graphics, StaticBufferIndex bufferIndex);

	UINT64 GetBufferOffsetAtIndex(const HeapData& heapData, unsigned int bufferIndex);

protected:
	bool m_finished = false;

	// non static resources - on shared memory; size multiplied by n frames-in-flight
	HeapData m_dynamicHeap;

	// static resources - on GPU-only memory
	HeapData m_staticHeap;

	void* pBufferHeapMappedData = nullptr;
	
	// temp resources
	// each buffer is 256 bytes
	UINT64 m_numberOfTempBuffers = 1024;
	UINT64 m_numTempBuffersUsed = 0;

	// struct contaning data for updating resource on GPU
	struct UploadResource 
	{
		std::unique_ptr<GraphicsBuffer> uploadResource;
		unsigned int workRangeInBytes;
		StaticBufferIndex staticResourceID = StaticBufferIndex();
	};

	std::vector<UploadResource> m_uploadResources;

	struct FrequentlyUpdatedResourceData
	{
		void* data;
		size_t dataSize;

		unsigned int frameIndex;
		bool updated = false;
	};

	std::unordered_map<unsigned int, FrequentlyUpdatedResourceData> m_frequentlyUpdatedResourcesToUpdate;
};

class ConstantBufferHeap : public BufferHeapBase
{
public:  // At program initialization
	TempBufferIndex GetNextTempIndex(UINT resourceSize);
	StaticBufferIndex RequestMoreStaticSpace(UINT resourceSize);
	DynamicBufferIndex RequestMoreSpace(Graphics& graphics, UINT resourceSize);

protected:
	UINT64 GetAligned(UINT64 offset, UINT stride);
};

class BufferHeap : public BufferHeapBase
{
public:
	BufferHeap();

public:  // At program initialization
	StaticBufferIndex RequestMoreStaticSpace(UINT resourceSize, UINT stride);
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