#include "InputLayout.h"
#include "Macros/ErrorMacros.h"
#include "PipelineState.h"
#include "DynamicVertex.h"

InputLayout::InputLayout(DynamicVertex::DynamicVertexLayout& layout)
	:
	m_layoutElements(layout.GetInputLayout())
{
	THROW_OBJECT_STATE_ERROR_IF("Input layout cannot exceed 15 elements", layout.GetNumElements() > 15);

	m_desc = {};
	m_desc.pInputElementDescs = m_layoutElements.data();
	m_desc.NumElements = m_layoutElements.size();
}

D3D12_INPUT_LAYOUT_DESC InputLayout::Get()
{
	return m_desc;
}

void InputLayout::BindToPipelineState(Graphics& graphics, PipelineState* pipelineState)
{
	pipelineState->SetInputLayout(this);
}