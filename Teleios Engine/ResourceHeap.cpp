#include "ResourceHeap.h"
#include "Graphics.h"
#include "Macros/ErrorMacros.h"

void ResourceHeap::RequestMoreGPUSpace(UINT resourceAlignedSize)
{
	m_requestedGPUSize += resourceAlignedSize;
}

void ResourceHeap::RequestMoreSharedSpace(UINT resourceAlignedSize)
{
	m_requestedSharedSize += resourceAlignedSize;
}

void ResourceHeap::Finish(Graphics& graphics)
{
	HRESULT hr;

	// Creating heap for GPU obejcts
	{
		D3D12_HEAP_PROPERTIES heapPropeties = {};
		heapPropeties.Type = D3D12_HEAP_TYPE_CUSTOM;
		heapPropeties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_NOT_AVAILABLE;
		heapPropeties.MemoryPoolPreference = D3D12_MEMORY_POOL_L1;
		heapPropeties.CreationNodeMask = 0;
		heapPropeties.VisibleNodeMask = 0;

		D3D12_HEAP_DESC heapDesc = {};
		heapDesc.SizeInBytes = m_requestedGPUSize;
		heapDesc.Properties = heapPropeties;
		heapDesc.Alignment = 0;
		heapDesc.Flags = D3D12_HEAP_FLAG_NONE;

		THROW_ERROR(graphics.GetDevice()->CreateHeap(
			&heapDesc,
			IID_PPV_ARGS(&pGPUHeap)
		));
	}

	// creating heap in shared memory, usually upload heaps
	{
		D3D12_HEAP_PROPERTIES heapPropeties = {};
		heapPropeties.Type = D3D12_HEAP_TYPE_CUSTOM;
		heapPropeties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;
		heapPropeties.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;
		heapPropeties.CreationNodeMask = 0;
		heapPropeties.VisibleNodeMask = 0;

		D3D12_HEAP_DESC heapDesc = {};
		heapDesc.SizeInBytes = m_requestedSharedSize;
		heapDesc.Properties = heapPropeties;
		heapDesc.Alignment = 0;
		heapDesc.Flags = D3D12_HEAP_FLAG_NONE;

		THROW_ERROR(graphics.GetDevice()->CreateHeap(
			&heapDesc,
			IID_PPV_ARGS(&pSharedHeap)
		));
	}
}

UINT64 ResourceHeap::GetGPUOffset(Graphics& graphics, const D3D12_RESOURCE_DESC* thisResourceDesc)
{
	D3D12_RESOURCE_ALLOCATION_INFO resourceAllocationInfo = graphics.GetDevice()->GetResourceAllocationInfo(0, 0, thisResourceDesc);

	UINT64 previousOffset = m_nextGPUHandleOffset;

	m_nextGPUHandleOffset += resourceAllocationInfo.Alignment;

	return previousOffset;
}

UINT64 ResourceHeap::GetSharedOffset(Graphics& graphics, const D3D12_RESOURCE_DESC* thisResourceDesc)
{
	D3D12_RESOURCE_ALLOCATION_INFO resourceAllocationInfo = graphics.GetDevice()->GetResourceAllocationInfo(0, 0, thisResourceDesc);

	UINT64 previousOffset = m_nextSharedHandleOffset;

	m_nextSharedHandleOffset += resourceAllocationInfo.Alignment;

	return previousOffset;
}

ID3D12Heap* ResourceHeap::GetGPUHeap()
{
	return pGPUHeap.Get();
}

ID3D12Heap* ResourceHeap::GetSharedHeap()
{
	return pSharedHeap.Get();
}