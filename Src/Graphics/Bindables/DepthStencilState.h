#pragma once
#include "Includes/DirectXIncludes.h"
#include "Bindable.h"

class Graphics;
class GraphicsPipelineState;

enum class StencilOperation
{
	Keep,
	Zero,
	Replace,
	SaturateIncrease,
	SaturateDecrease,
	Invert,
	Increase,
	Decrease
};

enum class ComparisonFunction
{
	None,
	Never,
	Less,
	Equal,
	LessEqual,
	Greater,
	NotEqual,
	GreaterEqual,
	Always
};

enum class DepthWriteMask
{
	Zero,
	All
};

class DepthStencilOperationsOptions
{
public:
	std::string GetIdentifier() const;

public:
	StencilOperation GetStencilFailOperation() const;
	StencilOperation GetDepthFailOperation() const;
	StencilOperation GetPassOperation() const;
	ComparisonFunction GetStencilComparisonFunction() const;

public:
	void SetStencilFailOperation(StencilOperation stencilFail);
	void SetDepthFailOperation(StencilOperation depthFail);
	void SetPassOperation(StencilOperation pass);
	void SetStencilComparisonFunction(ComparisonFunction comparisonFunction);

private:
	StencilOperation m_stencilFail = StencilOperation::Keep;
	StencilOperation m_depthFail = StencilOperation::Keep;
	StencilOperation m_pass = StencilOperation::Replace;
	ComparisonFunction m_stencilComparisonFunction = ComparisonFunction::Less;
};

class DepthStencilStateOptions
{
public:
	std::string GetIdentifier() const;

public:
	bool GetDepthEnable() const;
	bool GetStencilEnable() const;
	DepthWriteMask GetDepthWriteMask() const;
	ComparisonFunction GetDepthComparisonFunction() const;
	unsigned char GetStencilReadMask() const;
	unsigned char GetStencilWriteMask() const;
	DepthStencilOperationsOptions GetFrontFaceOptions() const;
	DepthStencilOperationsOptions GetBackFaceOptions() const;

public:
	void SetDepthEnable(bool enable);
	void SetStencilEnable(bool enable);
	void SetDepthWriteMask(DepthWriteMask mask);
	void SetDepthComparisonFunction(ComparisonFunction comparisonFunction);
	void SetStencilReadMask(unsigned char mask);
	void SetStencilWriteMask(unsigned char mask);
	void SetFrontFaceOptions(DepthStencilOperationsOptions options);
	void SetBackFaceOptions(DepthStencilOperationsOptions options);

private:
	bool m_depthEnable = true;
	bool m_stencilEnable = false;
	DepthWriteMask m_depthWriteMask = DepthWriteMask::All;
	ComparisonFunction m_depthComparisonFunction = ComparisonFunction::Less;
	unsigned char m_stencilReadMask = 0xFF;
	unsigned char m_stencilWriteMask = 0xFF;
	DepthStencilOperationsOptions m_frontFaceOptions = {};
	DepthStencilOperationsOptions m_backFaceOptions = {};
};

class DepthStencilState : public Bindable, public PipelineStateBindable
{
public:
	DepthStencilState(Graphics& graphics, const DepthStencilStateOptions& options);

public:
	static std::shared_ptr<DepthStencilState> GetResource(Graphics& graphics, const DepthStencilStateOptions& options);

	static std::string GetIdentifier(const DepthStencilStateOptions& options);

public:
	D3D12_DEPTH_STENCIL_DESC Get();

	virtual void AddPipelineStateParam(Graphics& graphics, GraphicsPipelineStateParams* params) override;

	virtual BindableType GetBindableType() const override;

private:
	D3D12_DEPTH_STENCIL_DESC m_desc;
};