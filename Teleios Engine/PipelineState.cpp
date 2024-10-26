#include "PipelineState.h"
#include "Macros/ErrorMacros.h"
#include "Graphics.h"

#include "RootSignature.h"
#include "Shader.h"
#include "BlendState.h"
#include "RasterizerState.h"
#include "DepthStencilState.h"
#include "InputLayout.h"

#include "TargetShaders.h"

PipelineState::PipelineState()
{

}



void PipelineState::SetRootSignature(RootSignature* rootSignature)
{
	m_desc.pRootSignature = rootSignature->Get();
}

void PipelineState::SetShader(Shader* shader)
{
	D3D12_SHADER_BYTECODE* descShaderByteCode = GetShaderPointerValue(m_desc, shader->GetType());
	D3D12_SHADER_BYTECODE shaderByteCode = shader->GetShaderByteCode();

	descShaderByteCode->BytecodeLength = shaderByteCode.BytecodeLength;
	descShaderByteCode->pShaderBytecode = shaderByteCode.pShaderBytecode;
}

void PipelineState::SetBlendState(BlendState* blendState)
{
	m_desc.BlendState = blendState->Get();
}

void PipelineState::SetSampleMask(UINT sampleMask)
{
	m_desc.SampleMask = sampleMask;
}

void PipelineState::SetRasterizerState(RasterizerState* rasterizerState)
{
	m_desc.RasterizerState = rasterizerState->Get();
}

void PipelineState::SetDepthStencilState(DepthStencilState* depthStencilState)
{
	m_desc.DepthStencilState = depthStencilState->Get();
}

void PipelineState::SetInputLayout(InputLayout* inputLayout)
{
	m_desc.InputLayout = inputLayout->Get();
}

void PipelineState::SetPrimitiveTechnologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE technologyType)
{
	m_desc.PrimitiveTopologyType = technologyType;
}

void PipelineState::SetNumRenderTargets(UINT numRenderTargets)
{
	m_desc.NumRenderTargets = numRenderTargets;
}

void PipelineState::SetRenderTargetFormat(UINT index, DXGI_FORMAT renderTargetFormat)
{
	THROW_OBJECT_STATE_ERROR_IF("Index of render target formats exceeded", index > 7);
	THROW_OBJECT_STATE_ERROR_IF("Didn't set number of render targets", m_desc.NumRenderTargets == 0);
	THROW_OBJECT_STATE_ERROR_IF("Index of defined render targets exceeded", index > m_desc.NumRenderTargets - 1);

	m_desc.RTVFormats[index] = renderTargetFormat;
}

void PipelineState::SetDepthStencilFormat(DXGI_FORMAT depthStencilFormat)
{
	m_desc.DSVFormat = depthStencilFormat;
}

void PipelineState::SetSampleDesc(UINT count, UINT quality)
{
	m_desc.SampleDesc.Count = count;
	m_desc.SampleDesc.Quality = quality;
}


void PipelineState::Finish(Graphics& graphics)
{
	HRESULT hr;

	THROW_ERROR(graphics.GetDevice()->CreateGraphicsPipelineState(&m_desc, IID_PPV_ARGS(&pPipelineState)));

	m_finished = true;
}

ID3D12PipelineState* PipelineState::Get() const
{
	THROW_OBJECT_STATE_ERROR_IF("Tried to use unfinished Pipeline State", !m_finished);

	return pPipelineState.Get();
}

D3D12_SHADER_BYTECODE* PipelineState::GetShaderPointerValue(D3D12_GRAPHICS_PIPELINE_STATE_DESC& desc, ShaderType type)
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

		case ShaderType::ComputeShader:
			THROW_INTERNAL_ERROR("Graphic pipeline does not support compute shaders");
	}
}