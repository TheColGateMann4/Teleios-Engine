#include "Buffer.h"
#include "Macros/ErrorMacros.h"
#include "Graphics.h"
#include "CommandList.h"

Buffer::Buffer(Graphics& graphics, unsigned int bufferSize, DXGI_FORMAT format, CPUAccess cpuAccess, D3D12_RESOURCE_STATES state)
	:
	RootSignatureBindable({ {ShaderVisibilityGraphic::AllShaders, 0} }),
	m_format(format),
	m_size(bufferSize),
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
		resourceDesc.Width = bufferSize;
		resourceDesc.Height = 1;
		resourceDesc.DepthOrArraySize = 1;
		resourceDesc.MipLevels = 1;
		resourceDesc.Format = format;
		resourceDesc.SampleDesc.Count = 1;
		resourceDesc.SampleDesc.Quality = 0;
		resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

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

void Buffer::Update(Graphics& graphics, void* data, size_t size)
{
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

Microsoft::WRL::ComPtr<ID3D12Resource> Buffer::GetBuffer(Graphics & graphics)
{
	return m_pBuffer;
}

DXGI_FORMAT Buffer::GetFormat() const
{
	return m_format;
}

ID3D12Resource* Buffer::GetResource() const
{
	return m_pBuffer.Get();
}

size_t Buffer::GetSize() const
{
	return m_size;
}

void Buffer::BindToRootSignature(Graphics& graphics, RootSignature* rootSignature)
{
	THROW_INTERNAL_ERROR("Tried to bind UAV to graphic root signature");
}

void Buffer::BindToComputeRootSignature(Graphics& graphics, RootSignature* rootSignature)
{
	rootSignature->AddBufferParameter(this);
}

void Buffer::BindToCommandList(Graphics& graphics, CommandList* commandList)
{
	THROW_INTERNAL_ERROR("Tried to bind UAV to graphic command list");
}

void Buffer::BindToComputeCommandList(Graphics& graphics, CommandList* commandList)
{
	commandList->SetComputeDescriptorTable(graphics, this);
}

UINT Buffer::GetOffsetInDescriptor() const
{
	return m_descriptor.offsetInDescriptorFromStart;
}

D3D12_GPU_DESCRIPTOR_HANDLE Buffer::GetDescriptorHeapGPUHandle(Graphics& graphics) const
{
	return m_descriptor.descriptorHeapGpuHandle;
}

void Buffer::CopyResourcesTo(Graphics& graphics, CommandList* copyCommandList, Buffer* dst)
{
	THROW_INTERNAL_ERROR_IF("Dest resource was NULL", dst == nullptr);

	copyCommandList->SetResourceState(graphics, this, D3D12_RESOURCE_STATE_COPY_SOURCE);
	copyCommandList->SetResourceState(graphics, dst, D3D12_RESOURCE_STATE_COPY_DEST);

	copyCommandList->CopyResource(graphics, dst->GetResource(), this->GetResource());

	copyCommandList->SetResourceState(graphics, dst, dst->GetResourceTargetState());
	copyCommandList->SetResourceState(graphics, this, this->GetResourceTargetState());
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