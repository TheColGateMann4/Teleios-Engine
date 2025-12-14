#include "GraphicsResource.h"
#include "Macros/ErrorMacros.h"

#include "CommandList.h"

GraphicsResource::GraphicsResource(DXGI_FORMAT format, CPUAccess cpuAccess, D3D12_RESOURCE_STATES targetState)
	:
	m_format(format),
	m_cpuAccess(cpuAccess),
	m_state(D3D12_RESOURCE_STATE_COMMON),
	m_targetState(targetState)
{

}

ID3D12Resource* GraphicsResource::GetResource() const
{
	return m_pResource.Get();
}

void GraphicsResource::CopyResourcesTo(Graphics& graphics, CommandList* copyCommandList, GraphicsResource* dst)
{
	THROW_INTERNAL_ERROR_IF("Dest resource was NULL", dst == nullptr);

	BEGIN_COMMAND_LIST_EVENT(copyCommandList, "Copying GraphicsTexture");

	copyCommandList->SetResourceState(graphics, this, D3D12_RESOURCE_STATE_COPY_SOURCE);
	copyCommandList->SetResourceState(graphics, dst, D3D12_RESOURCE_STATE_COPY_DEST);

	copyCommandList->CopyResource(graphics, dst->GetResource(), this->GetResource());

	copyCommandList->SetResourceState(graphics, dst, dst->GetResourceTargetState());
	copyCommandList->SetResourceState(graphics, this, this->GetResourceTargetState());

	END_COMMAND_LIST_EVENT(copyCommandList);
}

DXGI_FORMAT GraphicsResource::GetFormat() const
{
	return m_format;
}

D3D12_RESOURCE_STATES GraphicsResource::GetResourceState() const
{
	return m_state;
}

D3D12_RESOURCE_STATES GraphicsResource::GetResourceTargetState()
{
	return m_targetState;
}

void GraphicsResource::SetResourceState(D3D12_RESOURCE_STATES newState)
{
	m_state = newState;
}

void GraphicsResource::SetTargetResourceState(D3D12_RESOURCE_STATES newState)
{
	m_targetState = newState;
}

GraphicsResource::CPUAccess GraphicsResource::GetCPUAccess() const
{
	return m_cpuAccess;
}

D3D12_CPU_PAGE_PROPERTY GraphicsResource::GetHardwareHeapUsagePropety(CPUAccess cpuAccess)
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

D3D12_MEMORY_POOL GraphicsResource::GetHardwareHeapMemoryPool(CPUAccess cpuAccess)
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

D3D12_HEAP_TYPE GraphicsResource::GetHardwareHeapType(CPUAccess cpuAccess)
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