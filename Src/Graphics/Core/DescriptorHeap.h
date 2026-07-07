#pragma once
#include "Includes/CppIncludes.h"
#include "Includes/WRLNoWarnings.h"
#include "Includes/DirectXIncludes.h"
#include "Includes/WRLNoWarnings.h"

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

public:
	void Initialize(Graphics& graphics); // allocates 1000 entries at the start

	void RequestMoreSpace(unsigned int space = 1);

	void Free(unsigned int descriptorOffset);

	DescriptorInfo GetHandle(unsigned int descriptorOffset = 0) const;

	DescriptorInfo GetNextHandle();
	
	ID3D12DescriptorHeap* Get() const;

	void Update(Graphics& graphics);

private:
	DescriptorInfo BuildDescriptorInfo(unsigned int index) const;

	void CreateHeaps(Graphics& graphics, unsigned int numDescriptors);

private:
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> pMasterHeap;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> pDescriptorHeap;

	unsigned int m_size = 0;
	unsigned int m_pendingGrowth = 0;
	unsigned int m_nextHandleIndex = 0;
	unsigned int m_descriptorIncrementSize = 0;
	bool m_initialized = false;

	std::vector<unsigned int> m_freeList;
};