#pragma once
#include "includes/CppIncludes.h"
#include "includes/DirectXIncludes.h"
#include "includes/WRLNoWarnings.h"

class Graphics;
class CommandList;

class ConstantBufferHeap
{
public: // At program initialization

	// returns index to constat buffer
	unsigned int RequestMoreSpace(Graphics& graphics, UINT resourceAlignedSize);
	unsigned int RequestMoreStaticSpace(Graphics& graphics, UINT resourceAlignedSize);

	void Finish(Graphics& graphics);

public:	// At runtime
	void CopyResources(Graphics& graphics, CommandList* copyCommandList);

	UINT64 GetFrameBufferOffsetAtIndex(Graphics& graphics, unsigned int bufferIndex);

	UINT64 GetOffsetAtIndex(unsigned int bufferIndex);
	UINT64 GetOffsetOfStaticBufferAtIndex(unsigned int bufferIndex);

	UINT64 GetBufferSizeAtIndex(Graphics& graphics, unsigned int bufferIndex);
	UINT64 GetStaticBufferSizeAtIndex(Graphics& graphics, unsigned int bufferIndex);

	D3D12_GPU_VIRTUAL_ADDRESS GetBufferAddress(Graphics& graphics, unsigned int bufferIndex);
	D3D12_GPU_VIRTUAL_ADDRESS GetStaticBufferAddress(unsigned int bufferIndex);

	void UpdateHeap(Graphics& graphics);

	void UpdateResource(Graphics& graphics, unsigned int bufferIndex, void* data, size_t size);
	void UpdateStaticResource(Graphics& graphics, unsigned int bufferIndex, void* data, size_t size);
	void UpdateFrequentlyUpdatedStaticResource(Graphics& graphics, unsigned int bufferIndex, void* data, size_t size);

private:
	// non static resources - on shared memory
	Microsoft::WRL::ComPtr<ID3D12Resource> pBufferHeap;
	std::vector<UINT64> m_bufferOffsets = {};
	UINT64 m_combinedSize = 0;

	// static resources - on GPU memory
	Microsoft::WRL::ComPtr<ID3D12Resource> pStaticBufferHeap;
	std::vector<UINT64> m_staticBufferOffsets = {};
	UINT64 m_combinedSizeStaticBuffer = 0;

	// struct contaning data for updating resource on GPU
	struct UploadResource 
	{
		Microsoft::WRL::ComPtr<ID3D12Resource> pUploadResource;
		unsigned int staticResourceID;
		unsigned int workRange;

		unsigned int usedAtFrameIndex;
		bool alreadyUsed = false;
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