#pragma once
#include "includes/CppIncludes.h"
#include "includes/DirectXIncludes.h"
#include "Bindable.h"

class Graphics;
class PipelineState;

namespace DynamicVertex
{
	class DynamicVertexLayout;
};

class InputLayout : public Bindable, public PipelineStateBindable
{
public:
	InputLayout(DynamicVertex::DynamicVertexLayout& layout);

	D3D12_INPUT_LAYOUT_DESC Get();

	virtual void BindToPipelineState(Graphics& graphics, PipelineState* pipelineState) override;

private:
	std::vector<D3D12_INPUT_ELEMENT_DESC> m_layoutElements;
	D3D12_INPUT_LAYOUT_DESC m_desc;
};

