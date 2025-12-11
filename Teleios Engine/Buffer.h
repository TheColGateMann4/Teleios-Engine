#pragma once
#include "includes/CppIncludes.h"
#include "includes/DirectXIncludes.h"
#include "includes/WRLNoWarnings.h"

#include "Bindable.h"
#include "DescriptorHeap.h"

class Graphics;
class Pipeline;
class RootSignature;
class CommandList;


class Buffer
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
	Buffer(Graphics& graphics, unsigned int numElements, unsigned int byteStride, CPUAccess cpuAccess = CPUAccess::notavailable, D3D12_RESOURCE_STATES state = D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE);

	Buffer(Buffer&&) noexcept = default;

public:
	void Update(Graphics& graphics, const void* data, size_t size);
	void Update(Graphics& graphics, Pipeline& pipeline, const void* data, size_t size);

	template<class T>
	void Update(Graphics& graphics, std::initializer_list<T> list)
	{
		Update(graphics, list.begin(), list.size() * sizeof(T));
	}

	template<class T>
	void Update(Graphics& graphics, Pipeline& pipeline, std::initializer_list<T> list)
	{
		Update(graphics, pipeline, list.begin(), list.size() * sizeof(T));
	}

	//virtual Microsoft::WRL::ComPtr<ID3D12Resource> GetBuffer(Graphics& graphics) = 0;
	Microsoft::WRL::ComPtr<ID3D12Resource> GetBuffer(Graphics& graphics);

	ID3D12Resource* GetResource() const;
	size_t GetByteSize() const;
	size_t GetNumElements() const;
	size_t GetByteStride() const;

	void CopyResourcesTo(Graphics& graphics, CommandList* copyCommandList, Buffer* dst);

	D3D12_RESOURCE_STATES GetResourceState() const;
	D3D12_RESOURCE_STATES GetResourceTargetState() const;
	void SetResourceState(D3D12_RESOURCE_STATES newState);

	CPUAccess GetCPUAccess() const;

private:
	void UpdateUsingTempResource(Graphics& graphics, Pipeline& pipeline, const void* data, size_t size);
	void UpdateLocalResource(Graphics& graphics, const void* data, size_t size);

	static D3D12_CPU_PAGE_PROPERTY GetHardwareHeapUsagePropety(CPUAccess cpuAccess);
	static D3D12_MEMORY_POOL GetHardwareHeapMemoryPool(CPUAccess cpuAccess);
	static D3D12_HEAP_TYPE GetHardwareHeapType(CPUAccess cpuAccess);

private:
	Microsoft::WRL::ComPtr<ID3D12Resource> m_pBuffer;
	DXGI_FORMAT m_format;
	size_t m_byteSize;
	size_t m_byteStride;
	size_t m_numElements;
	CPUAccess m_cpuAccess;
	D3D12_RESOURCE_STATES m_state;
	D3D12_RESOURCE_STATES m_targetState;
};

class SingleFrameBuffer
{


};

class MultiFrameBuffer
{


	// make buffer framesOfFlight times bigger
};