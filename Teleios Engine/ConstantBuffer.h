#pragma once
#include "includes/CppIncludes.h"
#include "includes/DirectXIncludes.h"
#include "includes/WRLNoWarnings.h"

class Graphics;

class ConstantBuffer
{
public:
	enum class ItemType
	{
		Int,
		Bool,
		Float,
		Float2,
		Float3,
		Float4,
		Matrix
	};

public:
	ConstantBuffer(Graphics& graphics, std::vector<ItemType> layout);

public:
	void SetData(Graphics& graphics, void* data, size_t size);

	void SetRootIndex(UINT rootIndex);

	UINT GetRootIndex() const;

	D3D12_GPU_VIRTUAL_ADDRESS GetGPUAddress() const;

private:
	static size_t GetAlignedSize(size_t currentSize, size_t nextElementSize, size_t alignment);
	static size_t GetAlignedSize(size_t currentSize, size_t alignment);
	
	static constexpr size_t GetElementSize(ItemType itemType);

private:
	size_t m_size;

	Microsoft::WRL::ComPtr<ID3D12Resource> pConstBuffer;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> pDescriptorHeap;
	UINT m_rootIndex = 0;
	bool m_initializedRootIndex = false;
};

