#include "InputLayout.h"
#include "Macros/ErrorMacros.h"
#include "PipelineState.h"
#include "DynamicVertex.h"

#include "BindableResourceList.h"

InputLayout::InputLayout(Graphics& graphics, DynamicVertex::DynamicVertexLayout& layout)
	:
	m_layoutElements(layout.GetInputLayout())
{
	THROW_OBJECT_STATE_ERROR_IF("Input layout cannot exceed 15 elements", layout.GetNumElements() > 15);

	m_desc = {};
	m_desc.pInputElementDescs = m_layoutElements.data();
	m_desc.NumElements = m_layoutElements.size();
}

std::shared_ptr<InputLayout> InputLayout::GetBindableResource(Graphics& graphics, DynamicVertex::DynamicVertexLayout& layout)
{
	return BindableResourceList::GetBindableResourceByID<InputLayout>(graphics, layout.GetIdentifier(), layout);
}

D3D12_INPUT_LAYOUT_DESC InputLayout::Get()
{
	return m_desc;
}

void InputLayout::BindToPipelineState(Graphics& graphics, GraphicsPipelineState* pipelineState)
{
	pipelineState->SetInputLayout(this);
}