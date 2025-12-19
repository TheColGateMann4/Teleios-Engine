#pragma once
#include "Includes/CppIncludes.h"
#include "Includes/DirectXIncludes.h"
#include "Includes/WRLNoWarnings.h"

class Graphics;
class CommandList;

class ConstantBufferHeap
{
public: // At program initialization

	// returns index to constat buffer
	unsigned int GetNextTempIndex(UINT resourceAlignedSize);
	unsigned int RequestMoreStaticSpace(UINT resourceAlignedSize);
	unsigned int RequestMoreSpace(Graphics& graphics, UINT resourceAlignedSize);

	void Finish(Graphics& graphics);

public:	// At runtime
	void CopyResources(Graphics& graphics, CommandList* copyCommandList);

	D3D12_GPU_VIRTUAL_ADDRESS GetTempBufferAddress(unsigned int bufferIndex);
	D3D12_GPU_VIRTUAL_ADDRESS GetBufferAddress(Graphics& graphics, unsigned int bufferIndex);
	D3D12_GPU_VIRTUAL_ADDRESS GetStaticBufferAddress(unsigned int bufferIndex);

	void UpdateHeap(Graphics& graphics);

	void UpdateTempResource(Graphics& graphics, unsigned int bufferIndex, void* data, size_t size);
	void UpdateResource(Graphics& graphics, unsigned int bufferIndex, void* data, size_t size);
	void UpdateStaticResource(Graphics& graphics, unsigned int bufferIndex, void* data, size_t size);
	void UpdateFrequentlyUpdatedStaticResource(Graphics& graphics, unsigned int bufferIndex, void* data, size_t size);

private:
	void UpdateResource(Graphics& graphics, UINT64 bufferStartingOffset, UINT64 bufferSize, void* data, size_t size);

private:
	UINT64 GetOffsetOfFrameBufferAtIndex(Graphics& graphics, unsigned int bufferIndex);
	UINT64 GetOffsetOfTempBufferAtIndex(unsigned int bufferIndex);
	UINT64 GetOffsetOfStaticBufferAtIndex(unsigned int bufferIndex);

	UINT64 GetOffsetAtIndex(unsigned int bufferIndex);

	UINT64 GetBufferSizeAtIndex(Graphics& graphics, unsigned int bufferIndex);
	UINT64 GetStaticBufferSizeAtIndex(Graphics& graphics, unsigned int bufferIndex);

private:
	bool m_finished = false;

	// non static resources - on shared memory
	Microsoft::WRL::ComPtr<ID3D12Resource> pBufferHeap;
	std::vector<UINT64> m_bufferOffsets = {};
	UINT64 m_combinedSize = 0;

	// static resources - on GPU memory
	Microsoft::WRL::ComPtr<ID3D12Resource> pStaticBufferHeap;
	std::vector<UINT64> m_staticBufferOffsets = {};
	UINT64 m_combinedSizeStaticBuffer = 0;
	
	// temp resources
	// each buffer is 256 bytes
	Microsoft::WRL::ComPtr<ID3D12Resource> pTempBufferHeap;
	UINT64 m_numberOfTempBuffers = 1024;
	UINT64 m_numTempBuffersUsed = 0;

	// struct contaning data for updating resource on GPU
	struct UploadResource 
	{
		Microsoft::WRL::ComPtr<ID3D12Resource> pUploadResource;
		unsigned int workRangeInBytes;

		unsigned int staticResourceID;
		unsigned int updatedAtFrameIndex;
		bool alreadyUpdated = false;
	};

	std::vector<UploadResource> m_uploadResources;

	struct FrequentlyUpdatedResourceData
	{
		void* data;
		size_t dataSize;

		unsigned int bufferIndex;
		unsigned int updatedAtFrameIndex;
		bool alreadyUpdated = false;
	};

	std::vector<FrequentlyUpdatedResourceData> m_frequentlyUpdatedResourcesToUpdate;
};