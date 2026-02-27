#pragma once
#include "Includes/DirectXIncludes.h"
#include "Bindable.h"

class Graphics;
class GraphicsPipelineState;

class RasterizerState : public Bindable, public PipelineStateBindable
{
public:
	RasterizerState(Graphics& graphics, bool twoSided = false);

public:
	static std::shared_ptr<RasterizerState> GetResource(Graphics& graphics, bool twoSided = false);

	static std::string GetIdentifier(bool twoSided);

public:
	D3D12_RASTERIZER_DESC Get();

	virtual void BindToPipelineState(Graphics& graphics, GraphicsPipelineState* pipelineState) override;

	virtual BindableType GetBindableType() const override;

private:
	D3D12_RASTERIZER_DESC m_desc;
};

