#include "ConstantBuffer.h"
#include "Macros/ErrorMacros.h"
#include "Graphics.h"

ConstantBuffer::ConstantBuffer(Graphics& graphics, std::vector<ItemType> layout)
	:
	m_size(0)
{
	// getting size of layout
	{
		for (const auto& element : layout)
		{
			m_size = GetAlignedSize(m_size, GetElementSize(element), 256);
			m_size += GetElementSize(element);
		}

		m_size = GetAlignedSize(m_size, 256);
	}

	HRESULT hr;

	// creating resource
	{
		D3D12_HEAP_PROPERTIES heapPropeties = {};
		heapPropeties.Type = D3D12_HEAP_TYPE_CUSTOM;
		heapPropeties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;
		heapPropeties.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;
		heapPropeties.VisibleNodeMask = 0;

		D3D12_RESOURCE_DESC resourceDesc = {};
		resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		resourceDesc.Alignment = 0;
		resourceDesc.Width = m_size;
		resourceDesc.Height = 1;
		resourceDesc.DepthOrArraySize = 1;
		resourceDesc.MipLevels = 1;
		resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
		resourceDesc.SampleDesc.Count = 1;
		resourceDesc.SampleDesc.Quality = 0;
		resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

		THROW_ERROR(graphics.GetDevice()->CreateCommittedResource(
			&heapPropeties,
			D3D12_HEAP_FLAG_NONE,
			&resourceDesc,
			D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,
			nullptr,
			IID_PPV_ARGS(&pConstBuffer)
		));
	}

	//creating decriptor heap
	{
		D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc = {};
		descriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		descriptorHeapDesc.NumDescriptors = 1;
		descriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		descriptorHeapDesc.NodeMask = 0;

		THROW_ERROR(graphics.GetDevice()->CreateDescriptorHeap(&descriptorHeapDesc, IID_PPV_ARGS(&pDescriptorHeap)));
	}

	// creating view
	{
		D3D12_CONSTANT_BUFFER_VIEW_DESC constBufferDesc = {};
		constBufferDesc.BufferLocation = pConstBuffer->GetGPUVirtualAddress();
		constBufferDesc.SizeInBytes = m_size;

		THROW_INFO_ERROR(graphics.GetDevice()->CreateConstantBufferView(
			&constBufferDesc,
			pDescriptorHeap->GetCPUDescriptorHandleForHeapStart()
		));
	}
}

void ConstantBuffer::SetData(Graphics& graphics, void* data, size_t size)
{
	THROW_INTERNAL_ERROR_IF("data passed to ConstantBuffer::SetData is invalid", size > m_size || data == nullptr);

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

		memcpy_s(pMappedData, m_size, data, size);

		pConstBuffer->Unmap(0, &writeRange);
	}
}

void ConstantBuffer::SetRootIndex(UINT rootIndex)
{
	m_initializedRootIndex = true;

	m_rootIndex = rootIndex;
}

UINT ConstantBuffer::GetRootIndex() const
{
	THROW_INTERNAL_ERROR_IF("Tried to get root index without setting it beforehand", !m_initializedRootIndex);

	return m_rootIndex;
}

D3D12_GPU_VIRTUAL_ADDRESS ConstantBuffer::GetGPUAddress() const
{
	return pConstBuffer->GetGPUVirtualAddress();
}

size_t ConstantBuffer::GetAlignedSize(size_t currentSize, size_t nextElementSize, size_t alignment)
{
	float numPacks = float(currentSize) / alignment; // number of packs so far we went through
	size_t sizeOfLastPack = (numPacks - std::floor(numPacks)) * alignment;
	size_t lastPackSizeWithNewElement = sizeOfLastPack + nextElementSize;
	
	return lastPackSizeWithNewElement > alignment ? std::ceil(numPacks) * alignment : currentSize;
}

size_t ConstantBuffer::GetAlignedSize(size_t currentSize, size_t alignment)
{
	float numPacks = float(currentSize) / alignment;
	
	return (numPacks != std::floor(numPacks)) ? std::ceil(numPacks) * alignment : currentSize;
}

constexpr size_t ConstantBuffer::GetElementSize(ItemType itemType)
{
	switch (itemType)
	{
		case ItemType::Int:
		case ItemType::Bool:
			return sizeof(int);

		case ItemType::Float:
			return sizeof(float);

		case ItemType::Float2:
			return sizeof(DirectX::XMFLOAT2);

		case ItemType::Float3:
			return sizeof(DirectX::XMFLOAT3);

		case ItemType::Float4:
			return sizeof(DirectX::XMFLOAT4);

		case ItemType::Matrix:
			sizeof(DirectX::XMMATRIX);

		default:
		{
			std::string errorString = "Item Type ";
			errorString += size_t(itemType);
			errorString += " wasn't defined in GetElementSize";

			THROW_INTERNAL_ERROR(errorString.c_str());
		}
	}
}