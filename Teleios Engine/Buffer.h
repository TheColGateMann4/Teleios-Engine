#pragma once
#include "includes/CppIncludes.h"
#include "includes/DirectXIncludes.h"
#include "includes/WRLNoWarnings.h"

#include "Bindable.h"
#include "DescriptorHeap.h"

class Graphics;
class RootSignature;
class CommandList;


class Buffer : public Bindable, public RootSignatureBindable, public CommandListBindable
{
public:
	enum class CPUAccess
	{
		readwrite,
		write,
		notavailable,
		unknown
	};
	
public:
	Buffer(Graphics& graphics, unsigned int bufferSize, DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN, CPUAccess cpuAccess = CPUAccess::notavailable, D3D12_RESOURCE_STATES state = D3D12_RESOURCE_STATE_COMMON);

public:
	void Update(Graphics& graphics, void* data, size_t size);

	//virtual Microsoft::WRL::ComPtr<ID3D12Resource> GetBuffer(Graphics& graphics) = 0;
	Microsoft::WRL::ComPtr<ID3D12Resource> GetBuffer(Graphics& graphics);

	DXGI_FORMAT GetFormat() const;

	ID3D12Resource* GetResource() const;
	size_t GetSize() const;

	virtual void BindToRootSignature(Graphics& graphics, RootSignature* rootSignature) override;

	virtual void BindToComputeRootSignature(Graphics& graphics, RootSignature* rootSignature) override;

	virtual void BindToCommandList(Graphics& graphics, CommandList* commandList) override;

	virtual void BindToComputeCommandList(Graphics& graphics, CommandList* commandList) override;

	UINT GetOffsetInDescriptor() const;

	virtual D3D12_GPU_DESCRIPTOR_HANDLE GetDescriptorHeapGPUHandle(Graphics& graphics) const override;

	void CopyResourcesTo(Graphics& graphics, CommandList* copyCommandList, Buffer* dst);

	D3D12_RESOURCE_STATES GetResourceState() const;
	D3D12_RESOURCE_STATES GetResourceTargetState() const;
	void SetResourceState(D3D12_RESOURCE_STATES newState);

	CPUAccess GetCPUAccess() const;

private:
	D3D12_CPU_PAGE_PROPERTY GetHardwareHeapUsagePropety(CPUAccess cpuAccess);
	D3D12_MEMORY_POOL GetHardwareHeapMemoryPool(CPUAccess cpuAccess);
	D3D12_HEAP_TYPE GetHardwareHeapType(CPUAccess cpuAccess);

private:
	Microsoft::WRL::ComPtr<ID3D12Resource> m_pBuffer;
	DXGI_FORMAT m_format;
	size_t m_size;
	CPUAccess m_cpuAccess;
	D3D12_RESOURCE_STATES m_state;
	D3D12_RESOURCE_STATES m_targetState;

	DescriptorHeap::DescriptorInfo m_descriptor = {};
};

class SingleFrameBuffer
{


};

class MultiFrameBuffer
{


	// make buffer framesOfFlight times bigger
};