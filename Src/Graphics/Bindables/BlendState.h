#pragma once
#include "Includes/DirectXIncludes.h"
#include "Bindable.h"

class Graphics;
class GraphicsPipelineState;

enum class BlendOperation
{
	Add,
	Substract,
	ReverseSubstract,
	Min,
	Max
};

enum class BlendOperationValue
{
	Zero,
	One,
	SrcColor,
	InvertedSrcColor,
	SrcAlpha,
	InvertedSrcAlpha,
	DestAlpha,
	InvertedDestAlpha,
	DestColor,
	InvertedDestColor,
	SaturatedSrcAlpha,
	BlendFactor,
	InversedBlendFactor,
	Src1Color,
	InversedSrc1Color,
	Src1Alpha,
	InversedSrc1Alpha,
	AlphaFactor,
	InversedAlphaFactor
};

enum class LogicOperation
{
	Clear,
	Set,
	Copy,
	CopyInverted,
	Noop,
	Invert,
	And,
	Nand,
	Or,
	Nor,
	Xor,
	Equal,
	AndDestInverted,
	AndSrcInverted,
	OrDestInverted,
	OrSrcInverted
};

enum class ColorWriteEnable
{
	Red,
	Green,
	Blue,
	Alpha,
	All
};

class BlendRenderTargetOptions
{
public:
	std::string GetIdentifier() const;

public:
	bool GetEnabled() const;
	bool GetLogicOperationEnabled() const;
	BlendOperationValue GetSrcBlend() const;
	BlendOperationValue GetSrcAlphaBlend() const;
	BlendOperationValue GetDestBlend() const;
	BlendOperationValue GetDestAlphaBlend() const;
	BlendOperation GetBlendOperation() const;
	BlendOperation GetBlendOperationAlpha() const;
	LogicOperation GetLogicOperation() const;
	ColorWriteEnable GetWriteMask() const;

public:
	void SetEnabled(bool enable);
	void SetLogicOperationEnabled(bool logicOperationEnable);
	void SetSrcBlend(BlendOperationValue srcBlend);
	void SetSrcAlphaBlend(BlendOperationValue srcAlphaBlend);
	void SetDestBlend(BlendOperationValue destBlend);
	void SetDestAlphaBlend(BlendOperationValue destAlphaBlend);
	void SetBlendOperation(BlendOperation blendOperation);
	void SetBlendOperationAlpha(BlendOperation blendOperationAlpha);
	void SetLogicOperation(LogicOperation logicOperation);
	void SetWriteMask(ColorWriteEnable writeMask);

private:
	bool m_enable = true;
	bool m_logicOperationEnable = false;
	BlendOperationValue m_srcBlend = BlendOperationValue::One;
	BlendOperationValue m_srcAlphaBlend = BlendOperationValue::One;
	BlendOperationValue m_destBlend = BlendOperationValue::Zero;
	BlendOperationValue m_destAlphaBlend = BlendOperationValue::Zero;
	BlendOperation m_blendOperation = BlendOperation::Add;
	BlendOperation m_blendOperationAlpha = BlendOperation::Add;
	LogicOperation m_logicOperation = LogicOperation::Noop;
	ColorWriteEnable m_writeMask = ColorWriteEnable::All;
};

class BlendStateOptions
{
public:
	std::string GetIdentifier() const;

public:
	bool GetAlphaToCoverage() const;
	bool GetIndependentBlend() const;
	const BlendRenderTargetOptions& GetRenderTargetOptions(unsigned int index = 0) const;

public:
	// alpha will be used for output color
	void SetAlphaToCoverage(bool alphaToCoverage);
	// every render target will use blend desc at index 0
	void SetIndependentBlend(bool independentBlend);
	void SetRenderTargetOptions(BlendRenderTargetOptions renderTargetOptions, unsigned int index = 0);

private:
	bool m_alphaToCoverage = false;	
	bool m_independentBlend = false;
	std::array<BlendRenderTargetOptions, 8> m_renderTargetOptions = {};
};

class BlendState : public Bindable, public PipelineStateBindable
{
public:
	BlendState(Graphics& graphics, const BlendStateOptions& options);

public:
	static std::shared_ptr<BlendState> GetResource(Graphics& graphics, const BlendStateOptions& options);

	static std::string GetIdentifier(const BlendStateOptions& options);

public:
	D3D12_BLEND_DESC Get();

	virtual void AddPipelineStateParam(Graphics& graphics, GraphicsPipelineStateParams* params) override;

	virtual BindableType GetBindableType() const override; 

private:
	D3D12_BLEND_DESC m_desc;
};