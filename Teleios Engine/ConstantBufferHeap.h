#pragma once
#include "includes/CppIncludes.h"
#include "includes/DirectXIncludes.h"
#include "includes/WRLNoWarnings.h"

class Graphics;

class ConstantBufferHeap
{
public: // At program initialization

	// returns index to constat buffer
	unsigned int RequestMoreSpace(Graphics& graphics, UINT resourceAlignedSize);

	void Finish(Graphics& graphics);

public:	// At runtime
	UINT64 GetFrameBufferOffsetAtIndex(Graphics& graphics, unsigned int bufferIndex);

	UINT64 GetOffsetAtIndex(unsigned int bufferIndex);

	UINT64 GetBufferSizeAtIndex(Graphics& graphics, unsigned int bufferIndex);

	D3D12_GPU_VIRTUAL_ADDRESS GetBufferAddress(Graphics& graphics, unsigned int bufferIndex);

	ID3D12Resource* Get();

	void Update(Graphics& graphics);

	void UpdateResource(Graphics& graphics, unsigned int bufferIndex, void* data, size_t size);

	void UpdateStaticResource(Graphics& graphics, unsigned int bufferIndex, void* data, size_t size);

private:
	Microsoft::WRL::ComPtr<ID3D12Resource> pConstBufferHeap;
	std::vector<UINT64> m_bufferOffsets = {};
	UINT64 m_combinedSize = 0;

	struct StatcResourceToUpdate
	{
		unsigned int resourceIndex;
		void* data;
		size_t dataSize;

		unsigned int frameIndex;
		bool alreadyUpdatedFirstTime;
	};

	std::vector<StatcResourceToUpdate> m_staticResourcesToUpdate;
};