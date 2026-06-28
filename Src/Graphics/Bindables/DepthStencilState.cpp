#include "DepthStencilState.h"
#include "Graphics/Core/PipelineState.h"

#include "Graphics/Core/ResourceList.h"

#include "Macros/ErrorMacros.h"

std::string DepthStencilOperationsOptions::GetIdentifier() const
{
	std::string result = {};

	result += std::to_string(static_cast<unsigned int>(m_stencilFail));
	result += '@';
	result += std::to_string(static_cast<unsigned int>(m_depthFail));
	result += '@';
	result += std::to_string(static_cast<unsigned int>(m_pass));
	result += '@';
	result += std::to_string(static_cast<unsigned int>(m_stencilComparisonFunction));
	result += '@';

	return result;
}

StencilOperation DepthStencilOperationsOptions::GetStencilFailOperation() const
{
	return m_stencilFail;
}

StencilOperation DepthStencilOperationsOptions::GetDepthFailOperation() const
{
	return m_depthFail;
}

StencilOperation DepthStencilOperationsOptions::GetPassOperation() const
{
	return m_pass;
}

ComparisonFunction DepthStencilOperationsOptions::GetStencilComparisonFunction() const
{
	return m_stencilComparisonFunction;
}

void DepthStencilOperationsOptions::SetStencilFailOperation(StencilOperation stencilFail)
{
	m_stencilFail = stencilFail;
}

void DepthStencilOperationsOptions::SetDepthFailOperation(StencilOperation depthFail)
{
	m_depthFail = depthFail;
}

void DepthStencilOperationsOptions::SetPassOperation(StencilOperation pass)
{
	m_pass = pass;
}

void DepthStencilOperationsOptions::SetStencilComparisonFunction(ComparisonFunction comparisonFunction)
{
	m_stencilComparisonFunction = comparisonFunction;
}

std::string DepthStencilStateOptions::GetIdentifier() const
{
	std::string result = {};

	result += std::to_string(m_depthEnable);
	result += '@';
	result += std::to_string(m_stencilEnable);
	result += '@';
	result += std::to_string(static_cast<unsigned int>(m_depthWriteMask));
	result += '@';
	result += std::to_string(static_cast<unsigned int>(m_depthComparisonFunction));
	result += '@';
	result += std::to_string(m_stencilReadMask);
	result += '@';
	result += std::to_string(m_stencilWriteMask);
	result += '@';
	result += m_frontFaceOptions.GetIdentifier();
	result += '@';
	result += m_backFaceOptions.GetIdentifier();
	result += '@';

	return result;
}

bool DepthStencilStateOptions::GetDepthEnable() const
{
	return m_depthEnable;
}

bool DepthStencilStateOptions::GetStencilEnable() const
{
	return m_stencilEnable;
}

DepthWriteMask DepthStencilStateOptions::GetDepthWriteMask() const
{
	return m_depthWriteMask;
}

ComparisonFunction DepthStencilStateOptions::GetDepthComparisonFunction() const
{
	return m_depthComparisonFunction;
}

unsigned char DepthStencilStateOptions::GetStencilReadMask() const
{
	return m_stencilReadMask;
}

unsigned char DepthStencilStateOptions::GetStencilWriteMask() const
{
	return m_stencilWriteMask;
}

DepthStencilOperationsOptions DepthStencilStateOptions::GetFrontFaceOptions() const
{
	return m_frontFaceOptions;
}

DepthStencilOperationsOptions DepthStencilStateOptions::GetBackFaceOptions() const
{
	return m_backFaceOptions;
}

void DepthStencilStateOptions::SetDepthEnable(bool enable)
{
	m_depthEnable = enable;
}

void DepthStencilStateOptions::SetStencilEnable(bool enable)
{
	m_stencilEnable = enable;
}

void DepthStencilStateOptions::SetDepthWriteMask(DepthWriteMask mask)
{
	m_depthWriteMask = mask;
}

void DepthStencilStateOptions::SetDepthComparisonFunction(ComparisonFunction comparisonFunction)
{
	m_depthComparisonFunction = comparisonFunction;
}

void DepthStencilStateOptions::SetStencilReadMask(unsigned char mask)
{
	m_stencilReadMask = mask;
}

void DepthStencilStateOptions::SetStencilWriteMask(unsigned char mask)
{
	m_stencilWriteMask = mask;
}

void DepthStencilStateOptions::SetFrontFaceOptions(DepthStencilOperationsOptions options)
{
	m_frontFaceOptions = options;
}

void DepthStencilStateOptions::SetBackFaceOptions(DepthStencilOperationsOptions options)
{
	m_backFaceOptions = options;
}

D3D12_DEPTH_WRITE_MASK GetDXWriteMask(DepthWriteMask mask)
{
	switch (mask)
	{
		case DepthWriteMask::Zero:
			return D3D12_DEPTH_WRITE_MASK_ZERO;
		case DepthWriteMask::All:
			return D3D12_DEPTH_WRITE_MASK_ALL;
		default:
			THROW_INTERNAL_ERROR("Failed to map write mask option to DX counterpart");
	}
}

D3D12_COMPARISON_FUNC GetDXComparisonFunction(ComparisonFunction comparisonFunction)
{
	switch (comparisonFunction)
	{
		case ComparisonFunction::None:
			return D3D12_COMPARISON_FUNC_NONE;
		case ComparisonFunction::Never:
			return D3D12_COMPARISON_FUNC_NEVER;
		case ComparisonFunction::Less:
			return D3D12_COMPARISON_FUNC_LESS;
		case ComparisonFunction::Equal:
			return D3D12_COMPARISON_FUNC_EQUAL;
		case ComparisonFunction::LessEqual:
			return D3D12_COMPARISON_FUNC_LESS_EQUAL;
		case ComparisonFunction::Greater:
			return D3D12_COMPARISON_FUNC_GREATER;
		case ComparisonFunction::NotEqual:
			return D3D12_COMPARISON_FUNC_NOT_EQUAL;
		case ComparisonFunction::GreaterEqual:
			return D3D12_COMPARISON_FUNC_GREATER_EQUAL;
		case ComparisonFunction::Always:
			return D3D12_COMPARISON_FUNC_ALWAYS;
		default:
			THROW_INTERNAL_ERROR("Failed to map write mask option to DX counterpart");
	}
}

D3D12_STENCIL_OP GetDXStencilOperation(StencilOperation stenciloperation)
{
	switch (stenciloperation)
	{
		case StencilOperation::Keep:
			return D3D12_STENCIL_OP_KEEP;
		case StencilOperation::Zero:
			return D3D12_STENCIL_OP_ZERO;
		case StencilOperation::Replace:
			return D3D12_STENCIL_OP_REPLACE;
		case StencilOperation::SaturateIncrease:
			return D3D12_STENCIL_OP_INCR_SAT;
		case StencilOperation::SaturateDecrease:
			return D3D12_STENCIL_OP_DECR_SAT;
		case StencilOperation::Invert:
			return D3D12_STENCIL_OP_INVERT;
		case StencilOperation::Increase:
			return D3D12_STENCIL_OP_INCR;
		case StencilOperation::Decrease:
			return D3D12_STENCIL_OP_DECR;
		default:
			THROW_INTERNAL_ERROR("Failed to map write mask option to DX counterpart");
	}
}

void SetStencilFaceDesc(D3D12_DEPTH_STENCILOP_DESC& face, const DepthStencilOperationsOptions& options)
{
	face.StencilFailOp = GetDXStencilOperation(options.GetStencilFailOperation());
	face.StencilDepthFailOp = GetDXStencilOperation(options.GetDepthFailOperation());
	face.StencilPassOp = GetDXStencilOperation(options.GetPassOperation());
	face.StencilFunc = GetDXComparisonFunction(options.GetStencilComparisonFunction());
}

DepthStencilState::DepthStencilState(Graphics& graphics, const DepthStencilStateOptions& options)
{
	m_desc = {};
	m_desc.DepthEnable = options.GetDepthEnable();
	m_desc.DepthWriteMask = GetDXWriteMask(options.GetDepthWriteMask());
	m_desc.DepthFunc = GetDXComparisonFunction(options.GetDepthComparisonFunction());
	m_desc.StencilEnable = options.GetStencilEnable();
	m_desc.StencilReadMask = options.GetStencilReadMask();
	m_desc.StencilWriteMask = options.GetStencilWriteMask();
	SetStencilFaceDesc(m_desc.FrontFace, options.GetFrontFaceOptions());
	SetStencilFaceDesc(m_desc.BackFace, options.GetBackFaceOptions());
}

std::shared_ptr<DepthStencilState> DepthStencilState::GetResource(Graphics& graphics, const DepthStencilStateOptions& options)
{
	return ResourceList::GetResource<DepthStencilState>(graphics, options);
}

std::string DepthStencilState::GetIdentifier(const DepthStencilStateOptions& options)
{
	std::string resultString = "DepthStencilState#";

	resultString += options.GetIdentifier();

	return resultString;
}

D3D12_DEPTH_STENCIL_DESC DepthStencilState::Get()
{
	return m_desc;
}

void DepthStencilState::AddPipelineStateParam(Graphics& graphics, GraphicsPipelineStateParams* params)
{
	params->SetDepthStencilState(this);
}

BindableType DepthStencilState::GetBindableType() const
{
	return BindableType::bindable_depthStencilState;
}