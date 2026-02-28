#pragma once
#include "Includes/DirectXIncludes.h"
#include "Bindable.h"

class Graphics;
class GraphicsPipelineState;

class BlendState : public Bindable, public PipelineStateBindable
{
public:
	BlendState(Graphics& graphics, bool alphaToCoverage = true);

public:
	static std::shared_ptr<BlendState> GetResource(Graphics& graphics, bool alphaToCoverage = true);

	static std::string GetIdentifier(bool alphaToCoverage);

public:
	D3D12_BLEND_DESC Get();

	virtual void AddPipelineStateParam(Graphics& graphics, GraphicsPipelineStateParams* params) override;

	virtual BindableType GetBindableType() const override; 

private:
	D3D12_BLEND_DESC m_desc;
};

