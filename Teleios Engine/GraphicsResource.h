#pragma once
#include "includes/CppIncludes.h"
#include "includes/DirectXIncludes.h"
#include "includes/WRLNoWarnings.h"

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
	GraphicsResource(DXGI_FORMAT format, CPUAccess cpuAccess, D3D12_RESOURCE_STATES targetState);

public:
	virtual ~GraphicsResource() = default;
	GraphicsResource(GraphicsResource&&) noexcept = default;

public:
	ID3D12Resource* GetResource() const;

	void CopyResourcesTo(Graphics& graphics, CommandList* copyCommandList, GraphicsResource* dst);

public:
	DXGI_FORMAT GetFormat() const;

	D3D12_RESOURCE_STATES GetResourceState() const;
	D3D12_RESOURCE_STATES GetResourceTargetState();
	void SetResourceState(D3D12_RESOURCE_STATES newState);
	void SetTargetResourceState(D3D12_RESOURCE_STATES newState);

	CPUAccess GetCPUAccess() const;

protected:
	static D3D12_CPU_PAGE_PROPERTY GetHardwareHeapUsagePropety(CPUAccess cpuAccess);
	static D3D12_MEMORY_POOL GetHardwareHeapMemoryPool(CPUAccess cpuAccess);
	static D3D12_HEAP_TYPE GetHardwareHeapType(CPUAccess cpuAccess);

protected:
	Microsoft::WRL::ComPtr<ID3D12Resource> m_pResource;
	DXGI_FORMAT m_format;
	CPUAccess m_cpuAccess;
	D3D12_RESOURCE_STATES m_state;
	D3D12_RESOURCE_STATES m_targetState;
};