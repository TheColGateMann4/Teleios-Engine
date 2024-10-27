#pragma once
#include "includes/CppIncludes.h"
#include "includes/DirectXIncludes.h"
#include "Bindable.h"

class Graphics;
class PipelineState;

class InputLayout : public Bindable, public PipelineStateBindable
{
public:
	enum class ItemType
	{
		Position3,
		Position4,
		Color3,
		Color4,
		TexCoords,
		Normal
	};

	struct Item
	{
		const char* semanticName;
		ItemType type;
	};

public:
	InputLayout(std::vector<Item>& layout);

	D3D12_INPUT_LAYOUT_DESC Get();

	virtual void BindToPipelineState(Graphics& graphics, PipelineState* pipelineState) override;

private:
	D3D12_INPUT_ELEMENT_DESC GetItemDesc(Item item,size_t accumulatedSize);

	size_t GetItemSize(ItemType itemType);
	DXGI_FORMAT GetItemFormat(ItemType itemType);

private:
	std::vector<D3D12_INPUT_ELEMENT_DESC> m_layoutElements;
	D3D12_INPUT_LAYOUT_DESC m_desc;
};

