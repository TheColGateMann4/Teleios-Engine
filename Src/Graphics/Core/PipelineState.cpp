#include "PipelineState.h"
#include "Macros/ErrorMacros.h"
#include "Graphics.h"

#include "Includes/BindablesInclude.h"

#include "RootSignature.h"

#include "Shaders/TargetShaders.h"

#include "Graphics/Core/ResourceList.h"


namespace
{
	D3D12_SHADER_BYTECODE* GetShaderPointerValue(D3D12_GRAPHICS_PIPELINE_STATE_DESC& desc, ShaderType type)
	{
		switch (type)
		{
		case ShaderType::PixelShader:
			return &desc.PS;
		case ShaderType::VertexShader:
			return &desc.VS;
		case ShaderType::HullShader:
			return &desc.HS;
		case ShaderType::DomainShader:
			return &desc.DS;
		case ShaderType::GeometryShader:
			return &desc.GS;

		default:
			return nullptr;
		}
	}

	template<class T>
	std::string GetStringFromEnum(T val)
	{
		static_assert(std::is_enum_v<T>);

		return std::to_string(static_cast<std::underlying_type_t<T>>(val));
	}

	template<class T>
	std::string GetStringFromFlags(T val)
	{
		static_assert(std::is_enum_v<T>);

		using UnderlyingType = std::underlying_type_t<T>;
		constexpr size_t bitCount = sizeof(UnderlyingType) * 8;

		auto underlyingValue = static_cast<UnderlyingType>(val);
		return std::bitset<bitCount>(underlyingValue).to_string();
	}

	std::string GetStringFromFloat(FLOAT val)
	{
		if (val == 0.0f)
			return "FLT_ZERO";
		else if (val == D3D12_FLOAT32_MAX)
			return "FLT_MAX";

		THROW_INTERNAL_ERROR("Unhandled floating point value");
	}
};

const D3D12_GRAPHICS_PIPELINE_STATE_DESC* GraphicsPipelineStateParams::GetDesc() const
{
	return &m_desc;
}

std::string GraphicsPipelineStateParams::GetIdentifier() const
{
	THROW_INTERNAL_ERROR_IF("GraphicsPipelineStateParams were not finished", !m_finished);

	return m_cachedIdentifier;
}

void GraphicsPipelineStateParams::Finish()
{
	CreateIdentifier();

	m_finished = true;
}

bool GraphicsPipelineStateParams::isFinished() const
{
	return m_finished;
}

void GraphicsPipelineStateParams::SetRootSignature(RootSignature* rootSignature)
{
	m_rootSignature = rootSignature;

	m_desc.pRootSignature = rootSignature->Get();
}

void GraphicsPipelineStateParams::SetShader(Shader* shader)
{
	ShaderType shaderType = shader->GetShaderType();

	THROW_INTERNAL_ERROR_IF("Graphic pipeline does not support compute shaders", shaderType == ShaderType::ComputeShader);

	m_shaders.push_back(shader);

	D3D12_SHADER_BYTECODE* descShaderByteCode = GetShaderPointerValue(m_desc, shaderType);
	D3D12_SHADER_BYTECODE shaderByteCode = shader->GetShaderByteCode();

	descShaderByteCode->BytecodeLength = shaderByteCode.BytecodeLength;
	descShaderByteCode->pShaderBytecode = shaderByteCode.pShaderBytecode;
}

void GraphicsPipelineStateParams::SetBlendState(BlendState* blendState)
{
	m_desc.BlendState = blendState->Get();
}

void GraphicsPipelineStateParams::SetSampleMask(UINT sampleMask)
{
	m_desc.SampleMask = sampleMask;
}

void GraphicsPipelineStateParams::SetRasterizerState(RasterizerState* rasterizerState)
{
	m_desc.RasterizerState = rasterizerState->Get();
}

void GraphicsPipelineStateParams::SetDepthStencilState(DepthStencilState* depthStencilState)
{
	m_desc.DepthStencilState = depthStencilState->Get();
}

void GraphicsPipelineStateParams::SetInputLayout(InputLayout* inputLayout)
{
	m_desc.InputLayout = inputLayout->Get();
}

void GraphicsPipelineStateParams::SetPrimitiveTechnologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE technologyType)
{
	m_desc.PrimitiveTopologyType = technologyType;
}

void GraphicsPipelineStateParams::SetNumRenderTargets(UINT numRenderTargets)
{
	m_desc.NumRenderTargets = numRenderTargets;
}

void GraphicsPipelineStateParams::SetRenderTargetFormat(UINT index, DXGI_FORMAT renderTargetFormat)
{
	THROW_OBJECT_STATE_ERROR_IF("Index of render target formats exceeded", index > 7);
	THROW_OBJECT_STATE_ERROR_IF("Didn't set number of render targets", m_desc.NumRenderTargets == 0);
	THROW_OBJECT_STATE_ERROR_IF("Index of defined render targets exceeded", index > m_desc.NumRenderTargets - 1);

	m_desc.RTVFormats[index] = renderTargetFormat;
}

void GraphicsPipelineStateParams::SetDepthStencilFormat(DXGI_FORMAT depthStencilFormat)
{
	m_desc.DSVFormat = depthStencilFormat;
}

void GraphicsPipelineStateParams::SetSampleDesc(UINT count, UINT quality)
{
	m_desc.SampleDesc.Count = count;
	m_desc.SampleDesc.Quality = quality;
}

std::string GetRenderTargetBlendIdentifier(const D3D12_RENDER_TARGET_BLEND_DESC& rtblend)
{
	std::string result = {};

	result += std::to_string(static_cast<bool>(rtblend.BlendEnable));
	result += std::to_string(static_cast<bool>(rtblend.LogicOpEnable));
	result += GetStringFromEnum(rtblend.SrcBlend);
	result += GetStringFromEnum(rtblend.DestBlend);
	result += GetStringFromEnum(rtblend.BlendOp);
	result += GetStringFromEnum(rtblend.SrcBlendAlpha);
	result += GetStringFromEnum(rtblend.DestBlendAlpha);
	result += GetStringFromEnum(rtblend.BlendOpAlpha);
	result += GetStringFromEnum(rtblend.LogicOp);
	result += std::to_string(rtblend.RenderTargetWriteMask);

	return result;
}

std::string GetBlendIdentifier(const D3D12_BLEND_DESC& blend)
{
	std::string result = {};

	THROW_INTERNAL_ERROR_IF("Unhandled: indepentent blend was enabled", blend.IndependentBlendEnable);

	result += std::to_string(blend.AlphaToCoverageEnable);
	result += std::to_string(blend.IndependentBlendEnable);
	result += GetRenderTargetBlendIdentifier(blend.RenderTarget[0]);

	return result;
}

std::string GetRasterizerIdentifier(const D3D12_RASTERIZER_DESC& rasterizer)
{
	std::string result = {};

	result += GetStringFromEnum(rasterizer.FillMode);
	result += GetStringFromEnum(rasterizer.CullMode);
	result += std::to_string(static_cast<bool>(rasterizer.FrontCounterClockwise));
	result += std::to_string(rasterizer.DepthBias);
	result += GetStringFromFloat(rasterizer.DepthBiasClamp);
	result += GetStringFromFloat(rasterizer.SlopeScaledDepthBias);
	result += std::to_string(static_cast<bool>(rasterizer.DepthClipEnable));
	result += std::to_string(static_cast<bool>(rasterizer.MultisampleEnable));
	result += std::to_string(static_cast<bool>(rasterizer.AntialiasedLineEnable));
	result += std::to_string(rasterizer.ForcedSampleCount);
	result += GetStringFromEnum(rasterizer.ConservativeRaster);

	return result;
}

std::string GetDepthStencilFaceIdentifier(const D3D12_DEPTH_STENCILOP_DESC& face)
{
	std::string result = {};

	result += GetStringFromEnum(face.StencilFailOp);
	result += GetStringFromEnum(face.StencilDepthFailOp);
	result += GetStringFromEnum(face.StencilPassOp);
	result += GetStringFromEnum(face.StencilFunc);

	return result;
}

std::string GetDepthStencilIdentifier(const D3D12_DEPTH_STENCIL_DESC& depthStencil)
{
	std::string result = {};

	result += std::to_string(static_cast<bool>(depthStencil.DepthEnable));
	result += GetStringFromEnum(depthStencil.DepthWriteMask);
	result += GetStringFromEnum(depthStencil.DepthFunc);
	result += std::to_string(static_cast<bool>(depthStencil.StencilEnable));
	result += std::to_string(depthStencil.StencilReadMask);
	result += std::to_string(depthStencil.StencilWriteMask);
	result += GetDepthStencilFaceIdentifier(depthStencil.FrontFace);
	result += GetDepthStencilFaceIdentifier(depthStencil.BackFace);

	return result;
}

std::string GetInputElementIdentifier(const D3D12_INPUT_ELEMENT_DESC& inputElement)
{
	std::string result = {};

	result += inputElement.SemanticName;
	result += std::to_string(inputElement.SemanticIndex);
	result += GetStringFromEnum(inputElement.Format);
	result += std::to_string(inputElement.InputSlot);
	result += std::to_string(inputElement.AlignedByteOffset);
	result += GetStringFromEnum(inputElement.InputSlotClass);
	result += std::to_string(inputElement.InstanceDataStepRate);

	return result;
}

std::string GetInputLayoutIdentifier(const D3D12_INPUT_LAYOUT_DESC& inputLayout)
{
	std::string result = {};

	result += std::to_string(inputLayout.NumElements);

	for (int i = 0; i < inputLayout.NumElements; i++)
		result += GetInputElementIdentifier(inputLayout.pInputElementDescs[i]);

	return result;
}

std::string GetSampleIdentifier(const DXGI_SAMPLE_DESC& sample)
{
	std::string result = {};

	result += std::to_string(sample.Count);
	result += std::to_string(sample.Quality);

	return result;
}

void GraphicsPipelineStateParams::CreateIdentifier()
{
	THROW_INTERNAL_ERROR_IF("GraphicsPipelineStateParams were not properly initialized", m_shaders.empty() || m_rootSignature == nullptr);

	std::string result = {};

	result += std::to_string(std::hash<std::string>{}(m_rootSignature->GetIdentifier()));

	for (const auto* shader : m_shaders)
		result += std::to_string(std::hash<std::wstring>{}(shader->GetPath()));

	result += GetBlendIdentifier(m_desc.BlendState);
	result += std::to_string(m_desc.SampleMask);
	result += GetRasterizerIdentifier(m_desc.RasterizerState);
	result += GetDepthStencilIdentifier(m_desc.DepthStencilState);
	result += GetInputLayoutIdentifier(m_desc.InputLayout);
	result += GetStringFromEnum(m_desc.IBStripCutValue);
	result += GetStringFromEnum(m_desc.PrimitiveTopologyType);
	result += std::to_string(m_desc.NumRenderTargets);
	result += GetStringFromEnum(m_desc.RTVFormats[0]);
	result += GetStringFromEnum(m_desc.DSVFormat);
	result += GetSampleIdentifier(m_desc.SampleDesc);
	result += GetStringFromFlags(m_desc.Flags);

	m_cachedIdentifier = result;
}

const D3D12_COMPUTE_PIPELINE_STATE_DESC* ComputePipelineStateParams::GetDesc() const
{
	return &m_desc;
}

std::string ComputePipelineStateParams::GetIdentifier() const
{
	THROW_INTERNAL_ERROR_IF("ComputePipelineStateParams were not finished", !m_finished);

	return m_cachedIdentifier;
}

void ComputePipelineStateParams::Finish()
{
	CreateIdentifier();

	m_finished = true;
}

bool ComputePipelineStateParams::isFinished() const
{
	return m_finished;
}

void ComputePipelineStateParams::SetRootSignature(RootSignature* rootSignature)
{
	m_rootSignature = rootSignature;
	m_desc.pRootSignature = rootSignature->Get();
}

void ComputePipelineStateParams::SetShader(Shader* shader)
{
	THROW_INTERNAL_ERROR_IF("Only compute shaders can be bound to compute pipeline state", shader->GetShaderType() != ShaderType::ComputeShader);

	m_computeShader = shader;

	m_desc.CS = shader->GetShaderByteCode();
}

void ComputePipelineStateParams::CreateIdentifier()
{
	THROW_INTERNAL_ERROR_IF("ComputePipelineStateParams were not properly initialized", m_rootSignature == nullptr || m_computeShader == nullptr);

	std::string result = {};

	result += std::to_string(std::hash<std::string>{}(m_rootSignature->GetIdentifier()));
	result += std::to_string(std::hash<std::wstring>{}(m_computeShader->GetPath()));

	m_cachedIdentifier = result;
}

ID3D12PipelineState* PipelineState::Get() const
{
	return pPipelineState.Get();
}

GraphicsPipelineState::GraphicsPipelineState(Graphics& graphics, GraphicsPipelineStateParams&& params)
	:
	m_params(std::move(params))
{
	if (!m_params.isFinished())
		m_params.Finish();

	HRESULT hr;

	THROW_ERROR(graphics.GetDeviceResources().GetDevice()->CreateGraphicsPipelineState(m_params.GetDesc(), IID_PPV_ARGS(&pPipelineState)));
}

std::shared_ptr<GraphicsPipelineState> GraphicsPipelineState::GetResource(Graphics& graphics, GraphicsPipelineStateParams&& params)
{
	if (!params.isFinished())
		params.Finish();

	return ResourceList::GetResource<GraphicsPipelineState>(graphics, std::move(params));
}

std::string GraphicsPipelineState::GetIdentifier(const GraphicsPipelineStateParams& params)
{
	return params.GetIdentifier();
}

ComputePipelineState::ComputePipelineState(Graphics& graphics, ComputePipelineStateParams&& params)
	:
	m_params(std::move(params))
{
	if (!m_params.isFinished())
		m_params.Finish();

	HRESULT hr;

	THROW_ERROR(graphics.GetDeviceResources().GetDevice()->CreateComputePipelineState(m_params.GetDesc(), IID_PPV_ARGS(&pPipelineState)));
}

std::shared_ptr<ComputePipelineState> ComputePipelineState::GetResource(Graphics& graphics, ComputePipelineStateParams&& params)
{
	if (!params.isFinished())
		params.Finish();

	return ResourceList::GetResource<ComputePipelineState>(graphics, std::move(params));
}

std::string ComputePipelineState::GetIdentifier(const ComputePipelineStateParams& params)
{
	return params.GetIdentifier();
}