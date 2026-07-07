#include "Graphics/Core/DescriptorHeap.h"
#include "Graphics.h"
#include "Macros/ErrorMacros.h"

#define ADDITIONAL_DESCRIPTOR_HEAP_SIZE 4096

void DescriptorHeap::Initialize(Graphics& graphics)
{
	THROW_OBJECT_STATE_ERROR_IF("Tried to initialize DescriptorHeap twice", m_initialized);

	m_size = ADDITIONAL_DESCRIPTOR_HEAP_SIZE;
	CreateHeaps(graphics, m_size);

	m_descriptorIncrementSize = graphics.GetDeviceResources().GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	m_initialized = true;
}

void DescriptorHeap::RequestMoreSpace(unsigned int space)
{
	THROW_OBJECT_STATE_ERROR_IF("Tried to request space before DescriptorHeap was initialized", !m_initialized);

	m_pendingGrowth += space;
}

void DescriptorHeap::Free(unsigned int descriptorOffset)
{
	THROW_OBJECT_STATE_ERROR_IF("Tried to free a descriptor before DescriptorHeap was initialized", !m_initialized);
	THROW_INTERNAL_ERROR_IF("Tried to free a descriptor index that was never allocated", descriptorOffset >= m_nextHandleIndex);

	m_freeList.push_back(descriptorOffset);
}

DescriptorHeap::DescriptorInfo DescriptorHeap::GetHandle(unsigned int descriptorOffset) const
{
	THROW_OBJECT_STATE_ERROR_IF("Tried to get a handle before DescriptorHeap was initialized", !m_initialized);
	THROW_INTERNAL_ERROR_IF("Tried to get descriptor handle out of bounds", descriptorOffset >= m_size);

	return BuildDescriptorInfo(descriptorOffset);
}

DescriptorHeap::DescriptorInfo DescriptorHeap::GetNextHandle()
{
	THROW_OBJECT_STATE_ERROR_IF("Tried to get a handle before DescriptorHeap was initialized", !m_initialized);

	if (!m_freeList.empty())
	{
		unsigned int index = m_freeList.back();
		m_freeList.pop_back();
		return BuildDescriptorInfo(index);
	}

	THROW_INTERNAL_ERROR_IF("Tried to get descriptor handle out of bounds", m_nextHandleIndex >= m_size);

	unsigned int index = m_nextHandleIndex;
	m_nextHandleIndex++;
	return BuildDescriptorInfo(index);
}

ID3D12DescriptorHeap* DescriptorHeap::Get() const
{
	return pDescriptorHeap.Get();
}

void DescriptorHeap::Update(Graphics& graphics)
{
	THROW_OBJECT_STATE_ERROR_IF("Tried to update DescriptorHeap before it was initialized", !m_initialized);

	unsigned int spareCapacity = m_size - m_nextHandleIndex;

	if (m_pendingGrowth <= spareCapacity)
		return;

	unsigned int newCapacity = m_size + m_pendingGrowth;
	ID3D12Device* device = graphics.GetDeviceResources().GetDevice();

	auto pOldMasterHeap = std::move(pMasterHeap);

	CreateHeaps(graphics, newCapacity);

	if (m_nextHandleIndex > 0)
	{
		THROW_INFO_ERROR(device->CopyDescriptorsSimple(
			m_nextHandleIndex,
			pMasterHeap->GetCPUDescriptorHandleForHeapStart(),
			pOldMasterHeap->GetCPUDescriptorHandleForHeapStart(),
			D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)
		);

		THROW_INFO_ERROR(device->CopyDescriptorsSimple(
			m_nextHandleIndex,
			pDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
			pMasterHeap->GetCPUDescriptorHandleForHeapStart(),
			D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)
		);
	}

	m_size = newCapacity;
	m_pendingGrowth = 0;

	graphics.GetFrameResourceDeleter()->DeleteResource(graphics, std::move(pMasterHeap));
	graphics.GetFrameResourceDeleter()->DeleteResource(graphics, std::move(pDescriptorHeap));
}

DescriptorHeap::DescriptorInfo DescriptorHeap::BuildDescriptorInfo(unsigned int index) const
{
	SIZE_T resourceOffset = static_cast<SIZE_T>(m_descriptorIncrementSize) * index;

	DescriptorInfo descriptorInfo = {};
	descriptorInfo.descriptorCpuHandle = pDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	descriptorInfo.descriptorCpuHandle.ptr += resourceOffset;
	descriptorInfo.descriptorHeapGpuHandle = pDescriptorHeap->GetGPUDescriptorHandleForHeapStart();
	descriptorInfo.descriptorHeapGpuHandle.ptr += resourceOffset;
	descriptorInfo.offsetInDescriptorFromStart = index;
	return descriptorInfo;
}

void DescriptorHeap::CreateHeaps(Graphics& graphics, unsigned int numDescriptors)
{
	HRESULT hr;

	ID3D12Device* device = graphics.GetDeviceResources().GetDevice();

	D3D12_DESCRIPTOR_HEAP_DESC masterDesc = {};
	masterDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	masterDesc.NumDescriptors = numDescriptors;
	masterDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	masterDesc.NodeMask = 0;
	THROW_ERROR(device->CreateDescriptorHeap(&masterDesc, IID_PPV_ARGS(&pMasterHeap)));

	D3D12_DESCRIPTOR_HEAP_DESC visibleDesc = masterDesc;
	visibleDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	THROW_ERROR(device->CreateDescriptorHeap(&visibleDesc, IID_PPV_ARGS(&pDescriptorHeap)));
}