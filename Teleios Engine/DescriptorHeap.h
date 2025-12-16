#pragma once
#include "Includes/WRLNoWarnings.h"
#include "Includes/DirectXIncludes.h"

class Graphics;

class DescriptorHeap
{
public:
	struct DescriptorInfo
	{
		D3D12_CPU_DESCRIPTOR_HANDLE descriptorCpuHandle;
		D3D12_GPU_DESCRIPTOR_HANDLE descriptorHeapGpuHandle;
		UINT offsetInDescriptorFromStart;
	};

public: // At program initialization
	void RequestMoreSpace(unsigned int space = 1);

	void Finish(Graphics& graphics);

public:	// At runtime
	DescriptorInfo GetNextHandle();

	ID3D12DescriptorHeap* Get();

private:
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> pDescriptorHeap;
	UINT m_requestedSize = 0;
	UINT m_nextHandleIndex = 0;
	bool m_finished;

	UINT m_descriptorIncrementSize;
};