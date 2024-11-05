#pragma once
#include "includes/DirectXIncludes.h"
#include "Bindable.h"

class Graphics;
class PipelineState;

class RasterizerState : public Bindable, public PipelineStateBindable
{
public:
	RasterizerState();

public:
	static std::shared_ptr<RasterizerState> GetBindableResource();

	static std::string GetIdentifier();

public:
	D3D12_RASTERIZER_DESC Get();

	virtual void BindToPipelineState(Graphics& graphics, PipelineState* pipelineState) override;

private:
	D3D12_RASTERIZER_DESC m_desc;
};

