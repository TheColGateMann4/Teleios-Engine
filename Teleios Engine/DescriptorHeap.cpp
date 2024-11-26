#include "DescriptorHeap.h"
#include "Graphics.h"
#include "Macros/ErrorMacros.h"

void DescriptorHeap::RequestMoreSpace()
{
	THROW_OBJECT_STATE_ERROR_IF("Tried to increment DescriptorHeap size after it has been created", m_finished);

	m_requestedSize++;
}

void DescriptorHeap::Finish(Graphics& graphics)
{
	THROW_OBJECT_STATE_ERROR_IF("Tried to finish when DescriptorHeap object was finished", m_finished);
	THROW_OBJECT_STATE_ERROR_IF("Tried to finish when requested size was 0", m_requestedSize == 0);

	HRESULT hr;

	// creating descriptor heap object itself
	{
		D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc = {};
		descriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		descriptorHeapDesc.NumDescriptors = m_requestedSize;
		descriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		descriptorHeapDesc.NodeMask = 0;

		THROW_ERROR(graphics.GetDevice()->CreateDescriptorHeap(&descriptorHeapDesc, IID_PPV_ARGS(&pDescriptorHeap)));
	}

	m_descriptorIncrementSize = graphics.GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	m_finished = true;
}

DescriptorHeap::DescriptorInfo DescriptorHeap::GetNextHandle()
{
	THROW_OBJECT_STATE_ERROR_IF("Tried to get GPU handle when DescriptorHeap object wasn't finished yet", !m_finished);
	THROW_OBJECT_STATE_ERROR_IF("Tried to get descriptor handle out of bounds", m_nextHandleIndex > m_requestedSize - 1);

	SIZE_T resourceOffset = static_cast<SIZE_T>(m_descriptorIncrementSize * m_nextHandleIndex);

	DescriptorInfo descriptorInfo = {};

	descriptorInfo.descriptorCpuHandle = pDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	descriptorInfo.descriptorCpuHandle.ptr += resourceOffset;

	descriptorInfo.descriptorHeapGpuHandle = pDescriptorHeap->GetGPUDescriptorHandleForHeapStart();

	descriptorInfo.offsetInDescriptorFromStart = m_nextHandleIndex;

	m_nextHandleIndex++;

	return descriptorInfo;
}

ID3D12DescriptorHeap* DescriptorHeap::Get()
{
	return pDescriptorHeap.Get();
}