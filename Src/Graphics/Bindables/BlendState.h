#pragma once
#include "Includes/DirectXIncludes.h"
#include "Bindable.h"

class Graphics;
class GraphicsPipelineState;

class BlendState : public Bindable, public PipelineStateBindable
{
public:
	BlendState(Graphics& graphics);

public:
	static std::shared_ptr<BlendState> GetBindableResource(Graphics& graphics);

	static std::string GetIdentifier();

public:
	D3D12_BLEND_DESC Get();

	virtual void BindToPipelineState(Graphics& graphics, GraphicsPipelineState* pipelineState) override;

	virtual BindableType GetBindableType() const override; 

private:
	D3D12_BLEND_DESC m_desc;
};

