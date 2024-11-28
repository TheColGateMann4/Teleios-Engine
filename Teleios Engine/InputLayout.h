#pragma once
#include "includes/CppIncludes.h"
#include "includes/DirectXIncludes.h"
#include "Bindable.h"

class Graphics;
class GraphicsPipelineState;

namespace DynamicVertex
{
	class DynamicVertexLayout;
};

class InputLayout : public Bindable, public PipelineStateBindable
{
public:
	InputLayout(Graphics& graphics, DynamicVertex::DynamicVertexLayout& layout);

public:
	static std::shared_ptr<InputLayout> GetBindableResource(Graphics& graphics, DynamicVertex::DynamicVertexLayout& layout);

public:
	D3D12_INPUT_LAYOUT_DESC Get();

	virtual void BindToPipelineState(Graphics& graphics, GraphicsPipelineState* pipelineState) override;

private:
	std::vector<D3D12_INPUT_ELEMENT_DESC> m_layoutElements;
	D3D12_INPUT_LAYOUT_DESC m_desc;
};

