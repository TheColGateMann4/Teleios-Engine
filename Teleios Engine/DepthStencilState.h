#pragma once
#include "includes/DirectXIncludes.h"
#include "Bindable.h"

class Graphics;
class PipelineState;

class DepthStencilState : public Bindable, public PipelineStateBindable
{
public:
	DepthStencilState(Graphics& graphics);

public:
	static std::shared_ptr<DepthStencilState> GetBindableResource(Graphics& graphics);

	static std::string GetIdentifier();

public:
	D3D12_DEPTH_STENCIL_DESC Get();

	virtual void BindToPipelineState(Graphics& graphics, PipelineState* pipelineState) override;

private:
	D3D12_DEPTH_STENCIL_DESC m_desc;
};