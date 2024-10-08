#pragma once
#include "includes/CppIncludes.h"
#include "includes/DirectXIncludes.h"

class InputLayout
{
public:
	enum class ItemType
	{
		Position3,
		Position4,
		Color3,
		Color4
	};

	struct Item
	{
		const char* semanticName;
		ItemType type;
	};

public:
	InputLayout(std::vector<Item>& layout);

	D3D12_INPUT_LAYOUT_DESC Get();

private:
	D3D12_INPUT_ELEMENT_DESC GetItemDesc(Item item,size_t accumulatedSize);

	size_t GetAlignedSize(size_t previousSize, size_t elementSize);
	size_t GetItemSize(ItemType itemType);
	DXGI_FORMAT GetItemFormat(ItemType itemType);

private:
	std::vector<D3D12_INPUT_ELEMENT_DESC> m_layoutElements;
	D3D12_INPUT_LAYOUT_DESC m_desc;
};

