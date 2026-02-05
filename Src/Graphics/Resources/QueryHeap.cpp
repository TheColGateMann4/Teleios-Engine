#include "QueryHeap.h"
#include "Macros/ErrorMacros.h"

#include "Graphics/Core/Graphics.h"

QueryHeap::QueryHeap(Graphics& graphics, unsigned int numEntries)
	:
	m_numElements(numEntries)
{
	HRESULT hr;

	D3D12_QUERY_HEAP_DESC queryHeapDesc = {};
	queryHeapDesc.Type = D3D12_QUERY_HEAP_TYPE_TIMESTAMP;
	queryHeapDesc.Count = numEntries;
	queryHeapDesc.NodeMask = 0;

	THROW_ERROR(graphics.GetDeviceResources().GetDevice()->CreateQueryHeap(
		&queryHeapDesc,
		IID_PPV_ARGS(&m_pResource))
	);
}

unsigned int QueryHeap::GetNumElements() const
{
	return m_numElements;
}

ID3D12QueryHeap* QueryHeap::Get() const
{
	return m_pResource.Get();
}