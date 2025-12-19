#pragma once
#include "Includes/CppIncludes.h"
#include "Includes/DirectXIncludes.h"
#include "Includes/WRLNoWarnings.h"

class Graphics;
class CommandList;

class GraphicsResource
{
public:
	enum class CPUAccess
	{
		readwrite,
		write,
		notavailable,
		unknown
	};

protected:
	struct ResourceStates
	{
		D3D12_RESOURCE_STATES currentState = D3D12_RESOURCE_STATE_COMMON;
		D3D12_RESOURCE_STATES targetState = D3D12_RESOURCE_STATE_COMMON;
	};

protected:
	GraphicsResource(DXGI_FORMAT format, CPUAccess cpuAccess, D3D12_RESOURCE_STATES targetState);

public:
	virtual ~GraphicsResource() = default;
	GraphicsResource(GraphicsResource&&) noexcept = default;

public:
	ID3D12Resource* GetResource() const;

	void CopyResourcesTo(Graphics& graphics, CommandList* copyCommandList, GraphicsResource* dst);

public:
	DXGI_FORMAT GetFormat() const;

	virtual D3D12_RESOURCE_STATES GetResourceState(unsigned int targetSubresource = 0) const;
	virtual D3D12_RESOURCE_STATES GetResourceTargetState(unsigned int targetSubresource = 0);
	virtual void SetAllResourceStates(D3D12_RESOURCE_STATES newState);
	virtual void SetResourceState(D3D12_RESOURCE_STATES newState, unsigned int targetSubresource = 0);
	virtual void SetTargetResourceState(D3D12_RESOURCE_STATES newState, unsigned int targetSubresource = 0);

	CPUAccess GetCPUAccess() const;

protected:
	static D3D12_CPU_PAGE_PROPERTY GetHardwareHeapUsagePropety(CPUAccess cpuAccess);
	static D3D12_MEMORY_POOL GetHardwareHeapMemoryPool(CPUAccess cpuAccess);
	static D3D12_HEAP_TYPE GetHardwareHeapType(CPUAccess cpuAccess);

protected:
	Microsoft::WRL::ComPtr<ID3D12Resource> m_pResource;
	DXGI_FORMAT m_format;
	CPUAccess m_cpuAccess;
	ResourceStates m_state;
};