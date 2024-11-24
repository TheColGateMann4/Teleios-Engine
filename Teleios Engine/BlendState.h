#pragma once
#include "includes/DirectXIncludes.h"
#include "Bindable.h"

class Graphics;
class PipelineState;

class BlendState : public Bindable, public PipelineStateBindable
{
public:
	BlendState(Graphics& graphics);

public:
	static std::shared_ptr<BlendState> GetBindableResource(Graphics& graphics);

	static std::string GetIdentifier();

public:
	D3D12_BLEND_DESC Get();

	virtual void BindToPipelineState(Graphics& graphics, PipelineState* pipelineState) override;

private:
	D3D12_BLEND_DESC m_desc;
};

