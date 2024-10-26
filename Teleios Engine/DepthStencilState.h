#pragma once
#include "includes/DirectXIncludes.h"
#include "Bindable.h"

class Graphics;
class PipelineState;

class DepthStencilState : public Bindable, public PipelineStateBindable
{
public:
	DepthStencilState();

	D3D12_DEPTH_STENCIL_DESC Get();

	virtual void BindToPipelineState(Graphics& graphics, PipelineState* pipelineState) override;

private:
	D3D12_DEPTH_STENCIL_DESC m_desc;
};