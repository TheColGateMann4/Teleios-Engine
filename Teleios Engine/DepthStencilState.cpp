#include "DepthStencilState.h"
#include "PipelineState.h"

#include "BindableResourceList.h"

DepthStencilState::DepthStencilState(Graphics& graphics)
{
	D3D12_DEPTH_STENCILOP_DESC frontFaceStencil = {};
	frontFaceStencil.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	frontFaceStencil.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	frontFaceStencil.StencilPassOp = D3D12_STENCIL_OP_REPLACE;
	frontFaceStencil.StencilFunc = D3D12_COMPARISON_FUNC_LESS;

	m_desc = {};
	m_desc.DepthEnable = true;
	m_desc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	m_desc.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	m_desc.StencilEnable = false;
	m_desc.StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK;
	m_desc.StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK;
	m_desc.FrontFace = frontFaceStencil;
	//m_desc.BackFace = ; // we don't use it anyways
}

std::shared_ptr<DepthStencilState> DepthStencilState::GetBindableResource(Graphics& graphics)
{
	return BindableResourceList::GetBindableResource<DepthStencilState>(graphics);
}

std::string DepthStencilState::GetIdentifier()
{
	std::string resultString = "DepthStencilState#";

	return resultString;
}

D3D12_DEPTH_STENCIL_DESC DepthStencilState::Get()
{
	return m_desc;
}

void DepthStencilState::BindToPipelineState(Graphics& graphics, PipelineState* pipelineState)
{
	pipelineState->SetDepthStencilState(this);
}