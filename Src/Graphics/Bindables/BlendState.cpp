#include "BlendState.h"
#include "Graphics/Core/PipelineState.h"
#include "Graphics/Core/ResourceList.h"

#include "Macros/ErrorMacros.h"

std::string BlendRenderTargetOptions::GetIdentifier() const
{
	std::string result = {};

	result += m_enable;
	result += '@';
	result += std::to_string(m_logicOperationEnable);
	result += '@';
	result += std::to_string(static_cast<unsigned int>(m_srcBlend));
	result += '@';
	result += std::to_string(static_cast<unsigned int>(m_srcAlphaBlend));
	result += '@';
	result += std::to_string(static_cast<unsigned int>(m_destBlend));
	result += '@';
	result += std::to_string(static_cast<unsigned int>(m_destAlphaBlend));
	result += '@';
	result += std::to_string(static_cast<unsigned int>(m_blendOperation));
	result += '@';
	result += std::to_string(static_cast<unsigned int>(m_blendOperationAlpha));
	result += '@';
	result += std::to_string(static_cast<unsigned int>(m_logicOperation));
	result += '@';
	result += std::to_string(static_cast<unsigned int>(m_writeMask));

	return result;
}

bool BlendRenderTargetOptions::GetEnabled() const
{
	return m_enable;
}

bool BlendRenderTargetOptions::GetLogicOperationEnabled() const
{
	return m_logicOperationEnable;
}

BlendOperationValue BlendRenderTargetOptions::GetSrcBlend() const
{
	return m_srcBlend;
}

BlendOperationValue BlendRenderTargetOptions::GetSrcAlphaBlend() const
{
	return m_srcAlphaBlend;
}

BlendOperationValue BlendRenderTargetOptions::GetDestBlend() const
{
	return m_destBlend;
}

BlendOperationValue BlendRenderTargetOptions::GetDestAlphaBlend() const
{
	return m_destAlphaBlend;
}

BlendOperation BlendRenderTargetOptions::GetBlendOperation() const
{
	return m_blendOperation;
}

BlendOperation BlendRenderTargetOptions::GetBlendOperationAlpha() const
{
	return m_blendOperationAlpha;
}

LogicOperation BlendRenderTargetOptions::GetLogicOperation() const
{
	return m_logicOperation;
}

ColorWriteEnable BlendRenderTargetOptions::GetWriteMask() const
{
	return m_writeMask;
}


void BlendRenderTargetOptions::SetEnabled(bool enable)
{
	m_enable = enable;
}

void BlendRenderTargetOptions::SetLogicOperationEnabled(bool logicOperationEnable)
{
	m_logicOperationEnable = logicOperationEnable;
}

void BlendRenderTargetOptions::SetSrcBlend(BlendOperationValue srcBlend)
{
	m_srcBlend = srcBlend;
}

void BlendRenderTargetOptions::SetSrcAlphaBlend(BlendOperationValue srcAlphaBlend)
{
	m_srcAlphaBlend = srcAlphaBlend;
}

void BlendRenderTargetOptions::SetDestBlend(BlendOperationValue destBlend)
{
	m_destBlend = destBlend;
}

void BlendRenderTargetOptions::SetDestAlphaBlend(BlendOperationValue destAlphaBlend)
{
	m_destAlphaBlend = destAlphaBlend;
}

void BlendRenderTargetOptions::SetBlendOperation(BlendOperation blendOperation)
{
	m_blendOperation = blendOperation;
}

void BlendRenderTargetOptions::SetBlendOperationAlpha(BlendOperation blendOperationAlpha)
{
	m_blendOperationAlpha = blendOperationAlpha;
}

void BlendRenderTargetOptions::SetLogicOperation(LogicOperation logicOperation)
{
	m_logicOperation = logicOperation;
}

void BlendRenderTargetOptions::SetWriteMask(ColorWriteEnable writeMask)
{
	m_writeMask = writeMask;
}

std::string BlendStateOptions::GetIdentifier() const
{
	std::string result = {};

	result += std::to_string(m_alphaToCoverage);
	result += '@';

	result += std::to_string(m_independentBlend);
	result += '@';

	if(m_independentBlend)
	{
		for (auto& renderTargetOption : m_renderTargetOptions)
		{
			result += renderTargetOption.GetIdentifier();
			result += '@';
		}
	}
	else
	{
		result += m_renderTargetOptions.front().GetIdentifier();
	}

	return result;
}

bool BlendStateOptions::GetAlphaToCoverage() const
{
	return m_alphaToCoverage;
}

bool BlendStateOptions::GetIndependentBlend() const
{
	return m_independentBlend;
}

const BlendRenderTargetOptions& BlendStateOptions::GetRenderTargetOptions(unsigned int index) const
{
	THROW_INTERNAL_ERROR_IF("Tried to access blend options out of bounds", index >= m_renderTargetOptions.size());
	THROW_INTERNAL_ERROR_IF("Tried to get unique RT blend option but indepented blending is OFF", index != 0 && !m_independentBlend);

	return m_renderTargetOptions.at(index);
}


void BlendStateOptions::SetAlphaToCoverage(bool alphaToCoverage)
{
	m_alphaToCoverage = alphaToCoverage;
}

void BlendStateOptions::SetIndependentBlend(bool independentBlend)
{
	m_independentBlend = independentBlend;
}

void BlendStateOptions::SetRenderTargetOptions(BlendRenderTargetOptions renderTargetOptions, unsigned int index)
{
	THROW_INTERNAL_ERROR_IF("Tried to set blend options out of bounds", index >= m_renderTargetOptions.size());
	THROW_INTERNAL_ERROR_IF("Tried to set unique RT blend option but indepented blending is OFF", index != 0 && !m_independentBlend);

	m_renderTargetOptions.at(index) = renderTargetOptions;
}

D3D12_BLEND GetDXBlendValue(BlendOperationValue val)
{
	switch (val)
	{	
		case BlendOperationValue::Zero:
			return D3D12_BLEND_ZERO;
		case BlendOperationValue::One:
			return D3D12_BLEND_ONE;
		case BlendOperationValue::SrcColor:
			return D3D12_BLEND_SRC_COLOR;
		case BlendOperationValue::InvertedSrcColor:
			return D3D12_BLEND_INV_SRC_COLOR;
		case BlendOperationValue::SrcAlpha:
			return D3D12_BLEND_SRC_ALPHA;
		case BlendOperationValue::InvertedSrcAlpha:
			return D3D12_BLEND_INV_SRC_ALPHA;
		case BlendOperationValue::DestAlpha:
			return D3D12_BLEND_DEST_ALPHA;
		case BlendOperationValue::InvertedDestAlpha:
			return D3D12_BLEND_INV_DEST_ALPHA;
		case BlendOperationValue::DestColor:
			return D3D12_BLEND_DEST_COLOR;
		case BlendOperationValue::InvertedDestColor:
			return D3D12_BLEND_INV_DEST_COLOR;
		case BlendOperationValue::SaturatedSrcAlpha:
			return D3D12_BLEND_SRC_ALPHA_SAT;
		case BlendOperationValue::BlendFactor:
			return D3D12_BLEND_BLEND_FACTOR;
		case BlendOperationValue::InversedBlendFactor:
			return D3D12_BLEND_INV_BLEND_FACTOR;
		case BlendOperationValue::Src1Color:
			return D3D12_BLEND_SRC1_COLOR;
		case BlendOperationValue::InversedSrc1Color:
			return D3D12_BLEND_INV_SRC1_COLOR;
		case BlendOperationValue::Src1Alpha:
			return D3D12_BLEND_SRC1_ALPHA;
		case BlendOperationValue::InversedSrc1Alpha:
			return D3D12_BLEND_INV_SRC1_ALPHA;
		case BlendOperationValue::AlphaFactor:
			return D3D12_BLEND_ALPHA_FACTOR;
		case BlendOperationValue::InversedAlphaFactor:
			return D3D12_BLEND_INV_ALPHA_FACTOR;
		default:
			THROW_INTERNAL_ERROR("Failed to map blend value option to DX counterpart");
	}
}

D3D12_BLEND_OP GetDXBlendOperation(BlendOperation blendOp)
{
	switch (blendOp)
	{
		case BlendOperation::Add:
			return D3D12_BLEND_OP_ADD;
		case BlendOperation::Substract:
			return D3D12_BLEND_OP_SUBTRACT;
		case BlendOperation::ReverseSubstract:
			return D3D12_BLEND_OP_REV_SUBTRACT;
		case BlendOperation::Min:
			return D3D12_BLEND_OP_MIN;
		case BlendOperation::Max:
			return D3D12_BLEND_OP_MAX;
		default:
			THROW_INTERNAL_ERROR("Failed to map blend operation option to DX counterpart");
	}
}

D3D12_LOGIC_OP GetDXLogicOperation(LogicOperation logicOp)
{
	switch (logicOp)
	{
		case LogicOperation::Clear:
			return D3D12_LOGIC_OP_CLEAR;
		case LogicOperation::Set:
			return D3D12_LOGIC_OP_SET;
		case LogicOperation::Copy:
			return D3D12_LOGIC_OP_COPY;
		case LogicOperation::CopyInverted:
			return D3D12_LOGIC_OP_COPY_INVERTED;
		case LogicOperation::Noop:
			return D3D12_LOGIC_OP_NOOP;
		case LogicOperation::Invert:
			return D3D12_LOGIC_OP_INVERT;
		case LogicOperation::And:
			return D3D12_LOGIC_OP_AND;
		case LogicOperation::Nand:
			return D3D12_LOGIC_OP_NAND;
		case LogicOperation::Or:
			return D3D12_LOGIC_OP_OR;
		case LogicOperation::Nor:
			return D3D12_LOGIC_OP_NOR;
		case LogicOperation::Xor:
			return D3D12_LOGIC_OP_XOR;
		case LogicOperation::Equal:
			return D3D12_LOGIC_OP_EQUIV;
		case LogicOperation::AndDestInverted:
			return D3D12_LOGIC_OP_AND_REVERSE;
		case LogicOperation::AndSrcInverted:
			return D3D12_LOGIC_OP_AND_INVERTED;
		case LogicOperation::OrDestInverted:
			return D3D12_LOGIC_OP_OR_REVERSE;
		case LogicOperation::OrSrcInverted:
			return D3D12_LOGIC_OP_OR_INVERTED;
		default:
			THROW_INTERNAL_ERROR("Failed to map logic operation option to DX counterpart");
	}
}

UINT8 GetDXMask(ColorWriteEnable writeOption)
{
	switch (writeOption)
	{
		case ColorWriteEnable::Red:
			return D3D12_COLOR_WRITE_ENABLE_RED;
		case ColorWriteEnable::Green:
			return D3D12_COLOR_WRITE_ENABLE_GREEN;
		case ColorWriteEnable::Blue:
			return D3D12_COLOR_WRITE_ENABLE_BLUE;
		case ColorWriteEnable::Alpha:
			return D3D12_COLOR_WRITE_ENABLE_ALPHA;
		case ColorWriteEnable::All:
			return D3D12_COLOR_WRITE_ENABLE_ALL;
		default:
			THROW_INTERNAL_ERROR("Failed to map write mask option to DX counterpart");
	}
}

void GetDXDescFromOptions(D3D12_RENDER_TARGET_BLEND_DESC& rtDesc, const BlendRenderTargetOptions& options)
{
	rtDesc.BlendEnable = options.GetEnabled();
	rtDesc.LogicOpEnable = options.GetLogicOperationEnabled();
	rtDesc.SrcBlend = GetDXBlendValue(options.GetSrcBlend());
	rtDesc.DestBlend = GetDXBlendValue(options.GetDestBlend());
	rtDesc.BlendOp = GetDXBlendOperation(options.GetBlendOperation());
	rtDesc.SrcBlendAlpha = GetDXBlendValue(options.GetSrcAlphaBlend());
	rtDesc.DestBlendAlpha = GetDXBlendValue(options.GetDestAlphaBlend());
	rtDesc.BlendOpAlpha = GetDXBlendOperation(options.GetBlendOperationAlpha());
	rtDesc.LogicOp = GetDXLogicOperation(options.GetLogicOperation());
	rtDesc.RenderTargetWriteMask = GetDXMask(options.GetWriteMask());
}

BlendState::BlendState(Graphics& graphics, const BlendStateOptions& options)
{
	m_desc.AlphaToCoverageEnable = options.GetAlphaToCoverage();
	m_desc.IndependentBlendEnable = options.GetIndependentBlend();

	if (m_desc.IndependentBlendEnable)
	{
		for(unsigned int i = 0; i < 8; i++)
			GetDXDescFromOptions(m_desc.RenderTarget[i], options.GetRenderTargetOptions(i));
	}
	else
	{
		GetDXDescFromOptions(m_desc.RenderTarget[0], options.GetRenderTargetOptions());
	}
}

std::shared_ptr<BlendState> BlendState::GetResource(Graphics& graphics, const BlendStateOptions& options)
{
	return ResourceList::GetResource<BlendState>(graphics, options);
}

std::string BlendState::GetIdentifier(const BlendStateOptions& options)
{
	std::string resultString = "BlendState#";

	resultString += options.GetIdentifier();

	return resultString;
}

D3D12_BLEND_DESC BlendState::Get()
{
	return m_desc;
}

void BlendState::AddPipelineStateParam(Graphics& graphics, GraphicsPipelineStateParams* params)
{
	params->SetBlendState(this);
}

BindableType BlendState::GetBindableType() const
{
	return BindableType::bindable_blendState;
}
