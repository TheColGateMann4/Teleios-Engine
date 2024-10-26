#include "BlendState.h"
#include "PipelineState.h"

BlendState::BlendState()
{
	D3D12_RENDER_TARGET_BLEND_DESC renderTargetBlendDesc = {};
	renderTargetBlendDesc.BlendEnable = true;
	renderTargetBlendDesc.LogicOpEnable = false;
	renderTargetBlendDesc.SrcBlend = D3D12_BLEND_ONE;
	renderTargetBlendDesc.DestBlend = D3D12_BLEND_ZERO;
	renderTargetBlendDesc.BlendOp = D3D12_BLEND_OP_ADD;
	renderTargetBlendDesc.SrcBlendAlpha = D3D12_BLEND_ONE;
	renderTargetBlendDesc.DestBlendAlpha = D3D12_BLEND_ZERO;
	renderTargetBlendDesc.BlendOpAlpha = D3D12_BLEND_OP_ADD;
	renderTargetBlendDesc.LogicOp = D3D12_LOGIC_OP_NOOP;
	renderTargetBlendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	m_desc = {};
	m_desc.AlphaToCoverageEnable = true;			// alpha will be used for output color
	m_desc.IndependentBlendEnable = false;			// every render target will use blend desc at index 0
	m_desc.RenderTarget[0] = renderTargetBlendDesc;
}

D3D12_BLEND_DESC BlendState::Get()
{
	return m_desc;
}

void BlendState::BindToPipelineState(Graphics& graphics, PipelineState* pipelineState)
{
	pipelineState->SetBlendState(this);
}