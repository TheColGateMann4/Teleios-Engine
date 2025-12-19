#include "PipelineState.h"
#include "Macros/ErrorMacros.h"
#include "Graphics.h"

#include "includes/BindablesInclude.h"

#include "RootSignature.h"

#include "TargetShaders.h"

/*
*			PipelineStateBase
*/ 

ID3D12PipelineState* PipelineState::Get() const
{
	THROW_OBJECT_STATE_ERROR_IF("Tried to use unfinished Pipeline State", !m_finished);

	return pPipelineState.Get();
}

/*
*			GraphicsPipelineState
*/

void GraphicsPipelineState::SetRootSignature(RootSignature* rootSignature)
{
	m_desc.pRootSignature = rootSignature->Get();
}

void GraphicsPipelineState::SetShader(Shader* shader)
{
	ShaderType shaderType = shader->GetType();

	THROW_INTERNAL_ERROR_IF("Graphic pipeline does not support compute shaders", shaderType == ShaderType::ComputeShader);

	D3D12_SHADER_BYTECODE* descShaderByteCode = GetShaderPointerValue(m_desc, shaderType);
	D3D12_SHADER_BYTECODE shaderByteCode = shader->GetShaderByteCode();

	descShaderByteCode->BytecodeLength = shaderByteCode.BytecodeLength;
	descShaderByteCode->pShaderBytecode = shaderByteCode.pShaderBytecode;
}

void GraphicsPipelineState::SetBlendState(BlendState* blendState)
{
	m_desc.BlendState = blendState->Get();
}

void GraphicsPipelineState::SetSampleMask(UINT sampleMask)
{
	m_desc.SampleMask = sampleMask;
}

void GraphicsPipelineState::SetRasterizerState(RasterizerState* rasterizerState)
{
	m_desc.RasterizerState = rasterizerState->Get();
}

void GraphicsPipelineState::SetDepthStencilState(DepthStencilState* depthStencilState)
{
	m_desc.DepthStencilState = depthStencilState->Get();
}

void GraphicsPipelineState::SetInputLayout(InputLayout* inputLayout)
{
	m_desc.InputLayout = inputLayout->Get();
}

void GraphicsPipelineState::SetPrimitiveTechnologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE technologyType)
{
	m_desc.PrimitiveTopologyType = technologyType;
}

void GraphicsPipelineState::SetNumRenderTargets(UINT numRenderTargets)
{
	m_desc.NumRenderTargets = numRenderTargets;
}

void GraphicsPipelineState::SetRenderTargetFormat(UINT index, DXGI_FORMAT renderTargetFormat)
{
	THROW_OBJECT_STATE_ERROR_IF("Index of render target formats exceeded", index > 7);
	THROW_OBJECT_STATE_ERROR_IF("Didn't set number of render targets", m_desc.NumRenderTargets == 0);
	THROW_OBJECT_STATE_ERROR_IF("Index of defined render targets exceeded", index > m_desc.NumRenderTargets - 1);

	m_desc.RTVFormats[index] = renderTargetFormat;
}

void GraphicsPipelineState::SetDepthStencilFormat(DXGI_FORMAT depthStencilFormat)
{
	m_desc.DSVFormat = depthStencilFormat;
}

void GraphicsPipelineState::SetSampleDesc(UINT count, UINT quality)
{
	m_desc.SampleDesc.Count = count;
	m_desc.SampleDesc.Quality = quality;
}


void GraphicsPipelineState::Finish(Graphics& graphics)
{
	HRESULT hr;

	THROW_ERROR(graphics.GetDevice()->CreateGraphicsPipelineState(&m_desc, IID_PPV_ARGS(&pPipelineState)));

	m_finished = true;
}

D3D12_SHADER_BYTECODE* GraphicsPipelineState::GetShaderPointerValue(D3D12_GRAPHICS_PIPELINE_STATE_DESC& desc, ShaderType type)
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

/*
*			ComputePipelineState
*/

void ComputePipelineState::SetRootSignature(RootSignature* rootSignature)
{
	m_desc.pRootSignature = rootSignature->Get();
}

void ComputePipelineState::SetShader(Shader* shader)
{
	THROW_INTERNAL_ERROR_IF("Only compute shaders can be bound to compute pipeline state", shader->GetType() != ShaderType::ComputeShader);

	m_desc.CS = shader->GetShaderByteCode();
}

void ComputePipelineState::Finish(Graphics& graphics)
{
	HRESULT hr;

	THROW_ERROR(graphics.GetDevice()->CreateComputePipelineState(&m_desc, IID_PPV_ARGS(&pPipelineState)));

	m_finished = true;
}