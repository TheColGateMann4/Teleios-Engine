#pragma once
#include "Includes/WRLNoWarnings.h"
#include "Includes/DirectXIncludes.h"

class Graphics;

class ResourceHeap
{
public: // At program initialization
	void RequestMoreGPUSpace(UINT resourceAlignedSize);
	void RequestMoreSharedSpace(UINT resourceAlignedSize);

	void Finish(Graphics& graphics);

public:	// At runtime
	UINT64 GetGPUOffset(Graphics& graphics, const D3D12_RESOURCE_DESC* thisResourceDesc);

	UINT64 GetSharedOffset(Graphics& graphics, const D3D12_RESOURCE_DESC* thisResourceDesc);

	ID3D12Heap* GetGPUHeap();

	ID3D12Heap* GetSharedHeap();

private:
	Microsoft::WRL::ComPtr<ID3D12Heap> pGPUHeap;
	Microsoft::WRL::ComPtr<ID3D12Heap> pSharedHeap;

	UINT64 m_requestedGPUSize = 0;
	UINT64 m_requestedSharedSize = 0;

	UINT64 m_nextGPUHandleOffset = 0;
	UINT64 m_nextSharedHandleOffset = 0;

	bool m_finished;
};