#include "GraphicsResource.h"
#include "Macros/ErrorMacros.h"

#include "Graphics/Core/Graphics.h"
#include "Graphics/Core/CommandList.h"

GraphicsResource::GraphicsResource(DXGI_FORMAT format, CPUAccess cpuAccess, D3D12_RESOURCE_STATES targetState)
	:
	m_format(format),
	m_cpuAccess(cpuAccess),
	m_state({ D3D12_RESOURCE_STATE_COMMON, targetState })
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

ResourceFootprint GraphicsResource::GetResourceFootprint(Graphics& graphics, unsigned int targetSubresource)
{
	ResourceFootprint footprint = {};

	D3D12_RESOURCE_DESC resourceDesc = m_pResource->GetDesc();

	THROW_INFO_ERROR(graphics.GetDeviceResources().GetDevice()->GetCopyableFootprints(
		&resourceDesc,
		targetSubresource,
		1,
		0,
		&footprint.layout,
		&footprint.numRows,
		&footprint.rowSizeInBytes,
		&footprint.totalBytes
	));

	return footprint;
}

DXGI_FORMAT GraphicsResource::GetFormat() const
{
	return m_format;
}

D3D12_RESOURCE_STATES GraphicsResource::GetResourceState(unsigned int targetSubresource) const
{
	return m_state.currentState;
}

D3D12_RESOURCE_STATES GraphicsResource::GetResourceTargetState(unsigned int targetSubresource)
{
	return m_state.targetState;
}

void GraphicsResource::SetAllResourceStates(D3D12_RESOURCE_STATES newState)
{
	m_state.currentState = newState;
}

void GraphicsResource::SetResourceState(D3D12_RESOURCE_STATES newState, unsigned int targetSubresource)
{
	THROW_INTERNAL_ERROR_IF("Tried to write to unkown a subresource", targetSubresource > 0);

	m_state.currentState = newState;
}

void GraphicsResource::SetTargetResourceState(D3D12_RESOURCE_STATES newState, unsigned int targetSubresource)
{
	THROW_INTERNAL_ERROR_IF("Tried to write to unkown a subresource", targetSubresource > 0);

	m_state.targetState = newState;
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