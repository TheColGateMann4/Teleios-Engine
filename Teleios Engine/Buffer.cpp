#include "Buffer.h"
#include "Macros/ErrorMacros.h"
#include "Graphics.h"
#include "Pipeline.h"
#include "CommandList.h"

Buffer::Buffer(Graphics& graphics, unsigned int numElements, unsigned int byteStride, CPUAccess cpuAccess, D3D12_RESOURCE_STATES state, D3D12_RESOURCE_FLAGS flags)
	:
	m_byteSize(numElements * byteStride),
	m_byteStride(byteStride),
	m_numElements(numElements),
	m_cpuAccess(cpuAccess),
	m_state(D3D12_RESOURCE_STATE_COMMON),
	m_targetState(state)
{
	HRESULT hr;
	unsigned int numberOfBuffers = graphics.GetBufferCount();
	
	// creating resource
	{
		D3D12_HEAP_PROPERTIES heapPropeties = {};
		heapPropeties.Type = GetHardwareHeapType(cpuAccess);
		heapPropeties.CPUPageProperty = GetHardwareHeapUsagePropety(cpuAccess);
		heapPropeties.MemoryPoolPreference = GetHardwareHeapMemoryPool(cpuAccess);
		heapPropeties.VisibleNodeMask = 0;

		D3D12_RESOURCE_DESC resourceDesc = {};
		resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		resourceDesc.Alignment = 0;
		resourceDesc.Width = m_byteSize;
		resourceDesc.Height = 1;
		resourceDesc.DepthOrArraySize = 1;
		resourceDesc.MipLevels = 1;
		resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
		resourceDesc.SampleDesc.Count = 1;
		resourceDesc.SampleDesc.Quality = 0;
		resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		resourceDesc.Flags = flags; 

		// this is very incorrect practice since creating many different commited resources for one purpose is bad practice. It is only temporary solution
		THROW_ERROR(graphics.GetDevice()->CreateCommittedResource(
			&heapPropeties,
			D3D12_HEAP_FLAG_NONE,
			&resourceDesc,
			D3D12_RESOURCE_STATE_COMMON,
			nullptr,
			IID_PPV_ARGS(&m_pBuffer)
		));
	}
}

void Buffer::Update(Graphics& graphics, const void* data, size_t size)
{
	if (m_cpuAccess == CPUAccess::readwrite || m_cpuAccess == CPUAccess::write)
	{
		UpdateLocalResource(graphics, data, size);
	}
	else
	{
		THROW_INTERNAL_ERROR("Tried to Update resource without CPU access. Use pipeline access to update it using temp resource");
	}
}

void Buffer::Update(Graphics& graphics, Pipeline& pipeline, const void* data, size_t size)
{
	if (m_cpuAccess == CPUAccess::readwrite || m_cpuAccess == CPUAccess::write)
	{
		UpdateLocalResource(graphics, data, size);
	}
	else
	{
		UpdateUsingTempResource(graphics, pipeline, data, size);
	}
}

Microsoft::WRL::ComPtr<ID3D12Resource> Buffer::GetBuffer(Graphics & graphics)
{
	return m_pBuffer;
}

ID3D12Resource* Buffer::GetResource() const
{
	return m_pBuffer.Get();
}

size_t Buffer::GetByteSize() const
{
	return m_byteSize;
}

size_t Buffer::GetNumElements() const
{
	return m_numElements;
}

size_t Buffer::GetByteStride() const
{
	return m_byteStride;
}

void Buffer::CopyResourcesTo(Graphics& graphics, CommandList* copyCommandList, Buffer* dst)
{
	THROW_INTERNAL_ERROR_IF("Dest resource was NULL", dst == nullptr);

	BEGIN_COMMAND_LIST_EVENT(copyCommandList, "Copying Buffer");

	copyCommandList->SetResourceState(graphics, this, D3D12_RESOURCE_STATE_COPY_SOURCE);
	copyCommandList->SetResourceState(graphics, dst, D3D12_RESOURCE_STATE_COPY_DEST);

	copyCommandList->CopyResource(graphics, dst->GetResource(), this->GetResource());

	copyCommandList->SetResourceState(graphics, dst, dst->GetResourceTargetState());
	copyCommandList->SetResourceState(graphics, this, this->GetResourceTargetState());

	END_COMMAND_LIST_EVENT(copyCommandList);
}

D3D12_RESOURCE_STATES Buffer::GetResourceState() const
{
	return m_state;
}

D3D12_RESOURCE_STATES Buffer::GetResourceTargetState() const
{
	return m_targetState;
}

void Buffer::SetResourceState(D3D12_RESOURCE_STATES newState)
{
	m_state = newState;
}

Buffer::CPUAccess Buffer::GetCPUAccess() const
{
	return m_cpuAccess;
}

void Buffer::UpdateUsingTempResource(Graphics& graphics, Pipeline& pipeline, const void* data, size_t size)
{
	Buffer uploadBuffer(graphics, size, 1, CPUAccess::write);
	uploadBuffer.Update(graphics, data, size);

	uploadBuffer.CopyResourcesTo(graphics, pipeline.GetGraphicCommandList(), this);
}
void Buffer::UpdateLocalResource(Graphics& graphics, const void* data, size_t size)
{
	THROW_INTERNAL_ERROR_IF("Buffer was larger than resource itself", size > m_byteSize);

	Microsoft::WRL::ComPtr<ID3D12Resource> pConstBuffer = GetBuffer(graphics);

	HRESULT hr;

	// passing data to constant buffer resource
	{
		D3D12_RANGE readRange = {};
		readRange.Begin = 0;
		readRange.End = 0;

		D3D12_RANGE writeRange = {};
		writeRange.Begin = 0;
		writeRange.End = size;

		void* pMappedData = nullptr;

		THROW_ERROR(pConstBuffer->Map(
			0,
			&readRange,
			&pMappedData
		));

		memcpy_s(pMappedData, size, data, size);

		pConstBuffer->Unmap(0, &writeRange);
	}
}

D3D12_CPU_PAGE_PROPERTY Buffer::GetHardwareHeapUsagePropety(CPUAccess cpuAccess)
{
	switch (cpuAccess)
	{
		case CPUAccess::readwrite:		return D3D12_CPU_PAGE_PROPERTY_WRITE_COMBINE;
		case CPUAccess::write:			return D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;
		case CPUAccess::notavailable:	return D3D12_CPU_PAGE_PROPERTY_NOT_AVAILABLE;
		case CPUAccess::unknown:		return D3D12_CPU_PAGE_PROPERTY_UNKNOWN;

		default:  return D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	}
}

D3D12_MEMORY_POOL Buffer::GetHardwareHeapMemoryPool(CPUAccess cpuAccess)
{
	switch (cpuAccess)
	{
		case CPUAccess::readwrite:		return D3D12_MEMORY_POOL_L0;
		case CPUAccess::write:			return D3D12_MEMORY_POOL_L0;
		case CPUAccess::notavailable:	return D3D12_MEMORY_POOL_L1;
		case CPUAccess::unknown:		return D3D12_MEMORY_POOL_UNKNOWN;

		default:  return D3D12_MEMORY_POOL_UNKNOWN;
	}
}

D3D12_HEAP_TYPE Buffer::GetHardwareHeapType(CPUAccess cpuAccess)
{
	switch (cpuAccess)
	{
		case CPUAccess::readwrite:		return D3D12_HEAP_TYPE_CUSTOM;
		case CPUAccess::write:			return D3D12_HEAP_TYPE_CUSTOM;
		case CPUAccess::notavailable:	return D3D12_HEAP_TYPE_CUSTOM;
		case CPUAccess::unknown:		return D3D12_HEAP_TYPE_DEFAULT;

		default:  return D3D12_HEAP_TYPE_DEFAULT;
	}
}