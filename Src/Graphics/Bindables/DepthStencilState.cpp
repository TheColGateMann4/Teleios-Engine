#include "DepthStencilState.h"
#include "Graphics/Core/PipelineState.h"

#include "Graphics/Core/BindableResourceList.h"

#include "Macros/ErrorMacros.h"

DepthStencilState::DepthStencilState(Graphics& graphics, DepthComparisonFunc depthComparisonFunc)
{
	D3D12_DEPTH_STENCILOP_DESC frontFaceStencil = {};
	frontFaceStencil.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	frontFaceStencil.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	frontFaceStencil.StencilPassOp = D3D12_STENCIL_OP_REPLACE;
	frontFaceStencil.StencilFunc = D3D12_COMPARISON_FUNC_LESS;

	m_desc = {};
	m_desc.DepthEnable = true;
	m_desc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	m_desc.DepthFunc = GetComparisonFunc(depthComparisonFunc);
	m_desc.StencilEnable = false;
	m_desc.StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK;
	m_desc.StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK;
	m_desc.FrontFace = frontFaceStencil;
	m_desc.BackFace = frontFaceStencil;
}

std::shared_ptr<DepthStencilState> DepthStencilState::GetBindableResource(Graphics& graphics, DepthComparisonFunc depthComparisonFunc)
{
	return BindableResourceList::GetBindableResource<DepthStencilState>(graphics, depthComparisonFunc);
}

std::string DepthStencilState::GetIdentifier(DepthComparisonFunc depthComparisonFunc)
{
	std::string resultString = "DepthStencilState#";

	resultString += std::to_string(static_cast<unsigned int>(depthComparisonFunc));

	return resultString;
}

D3D12_DEPTH_STENCIL_DESC DepthStencilState::Get()
{
	return m_desc;
}

void DepthStencilState::BindToPipelineState(Graphics& graphics, GraphicsPipelineState* pipelineState)
{
	pipelineState->SetDepthStencilState(this);
}

BindableType DepthStencilState::GetBindableType() const
{
	return BindableType::bindable_depthStencilState;
}

D3D12_COMPARISON_FUNC DepthStencilState::GetComparisonFunc(DepthComparisonFunc depthComparisonFunc)
{
	switch (depthComparisonFunc)
	{
	case DepthComparisonFunc::Less:
		return D3D12_COMPARISON_FUNC_LESS;
	case DepthComparisonFunc::Equal:
		return D3D12_COMPARISON_FUNC_EQUAL;
	case DepthComparisonFunc::Greater:
		return D3D12_COMPARISON_FUNC_GREATER;

	default:
		THROW_INTERNAL_ERROR("Failed to map DepthComparisonFunc to DirectX function");
	}
}