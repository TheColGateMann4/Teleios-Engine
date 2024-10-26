#pragma once
#include "includes/DirectXIncludes.h"
#include "Bindable.h"

class Graphics;
class PipelineState;

class BlendState : public Bindable, public PipelineStateBindable
{
public:
	BlendState();

	D3D12_BLEND_DESC Get();

	virtual void BindToPipelineState(Graphics& graphics, PipelineState* pipelineState) override;

private:
	D3D12_BLEND_DESC m_desc;
};

