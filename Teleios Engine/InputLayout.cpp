#include "InputLayout.h"
#include "Macros/ErrorMacros.h"

InputLayout::InputLayout(std::vector<Item>& layout)
	:
	m_layoutElements()
{
	size_t layoutSize = layout.size();

	THROW_INTERNAL_ERROR_IF("Input layout cannot exceed 15 elements", layout.size() > 15);

	size_t accumulatedAlignedOffset = 0;

	for (const auto& item : layout)
	{
		m_layoutElements.push_back(GetItemDesc(item, accumulatedAlignedOffset));

		accumulatedAlignedOffset += GetItemSize(item.type);
	}

	m_desc = {};
	m_desc.pInputElementDescs = m_layoutElements.data();
	m_desc.NumElements = m_layoutElements.size();
}

D3D12_INPUT_LAYOUT_DESC InputLayout::Get()
{
	return m_desc;
}

D3D12_INPUT_ELEMENT_DESC InputLayout::GetItemDesc(Item item, size_t accumulatedSize)
{
	D3D12_INPUT_ELEMENT_DESC elementDesc = {};
	elementDesc.SemanticName = item.semanticName;
	elementDesc.SemanticIndex = 0;
	elementDesc.Format = GetItemFormat(item.type);
	elementDesc.InputSlot = 0;
	elementDesc.AlignedByteOffset = accumulatedSize;
	elementDesc.InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
	elementDesc.InstanceDataStepRate = 0;

	return elementDesc;
}

size_t InputLayout::GetItemSize(ItemType itemType)
{
	switch (itemType)
	{
		case ItemType::TexCoords:
			return sizeof(DirectX::XMFLOAT2);

		case ItemType::Position3:
		case ItemType::Color3:
			return sizeof(DirectX::XMFLOAT3);

		case ItemType::Position4:
		case ItemType::Color4:
			return sizeof(DirectX::XMFLOAT4);

		default:
		{
			std::string errorString = "Item Type ";
			errorString += size_t(itemType);
			errorString += " wasn't defined in GetItemSize";

			THROW_INTERNAL_ERROR(errorString.c_str());
		}
	}
}

DXGI_FORMAT InputLayout::GetItemFormat(ItemType itemType)
{
	switch(itemType)
	{
		case ItemType::TexCoords:
			return DXGI_FORMAT_R32G32_FLOAT;

		case ItemType::Position3:
		case ItemType::Color3:
			return DXGI_FORMAT_R32G32B32_FLOAT;

		case ItemType::Position4:
		case ItemType::Color4:
			return DXGI_FORMAT_R32G32B32A32_FLOAT;

		default:
		{
			std::string errorString = "Item Type ";
			errorString += size_t(itemType);
			errorString += " wasn't defined in GetItemFormat";

			THROW_INTERNAL_ERROR(errorString.c_str());
		}
	}
}