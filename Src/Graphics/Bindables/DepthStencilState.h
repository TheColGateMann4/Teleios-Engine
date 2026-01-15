#pragma once
#include "Includes/DirectXIncludes.h"
#include "Bindable.h"

class Graphics;
class GraphicsPipelineState;

class DepthStencilState : public Bindable, public PipelineStateBindable
{
public:
	enum class DepthComparisonFunc
	{
		Less,
		Equal,
		Greater
	};

public:
	DepthStencilState(Graphics& graphics, DepthComparisonFunc depthComparisonFunc = DepthComparisonFunc::Less);

public:
	static std::shared_ptr<DepthStencilState> GetBindableResource(Graphics& graphics, DepthComparisonFunc depthComparisonFunc = DepthComparisonFunc::Less);

	static std::string GetIdentifier(DepthComparisonFunc depthComparisonFunc);

public:
	D3D12_DEPTH_STENCIL_DESC Get();

	virtual void BindToPipelineState(Graphics& graphics, GraphicsPipelineState* pipelineState) override;

	virtual BindableType GetBindableType() const override;

private:
	D3D12_COMPARISON_FUNC GetComparisonFunc(DepthComparisonFunc depthComparisonFunc);

private:
	D3D12_DEPTH_STENCIL_DESC m_desc;
};