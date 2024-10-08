#include "PipelineState.h"
#include "Macros/ErrorMacros.h"
#include "Graphics.h"

#include "RootSignature.h"
#include "Shader.h"
#include "BlendState.h"
#include "RasterizerState.h"
#include "DepthStencilState.h"
#include "InputLayout.h"

PipelineState::PipelineState()
{

}



void PipelineState::SetRootSignature(RootSignature* rootSignature)
{
	m_desc.pRootSignature = rootSignature->Get();
}

void PipelineState::SetPixelShader(Shader* pixelShader)
{
	SetShaderData(pixelShader, &m_desc.PS);
}

void PipelineState::SetVertexShader(Shader* vertexShader)
{
	SetShaderData(vertexShader, &m_desc.VS);
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
	THROW_INTERNAL_ERROR_IF("index of render target formats exceeded", index > 7);
	THROW_INTERNAL_ERROR_IF("index of defined render target formats exceeded. Perhaps didn't set SetNumRenderTargets", index > m_desc.NumRenderTargets - 1);

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
	THROW_INTERNAL_ERROR_IF("Tried to use unfinished Pipeline State", !m_finished);

	return pPipelineState.Get();
}

void PipelineState::SetShaderData(Shader* shader, D3D12_SHADER_BYTECODE* shaderByteCode)
{
	ID3DBlob* blob = shader->GetShaderBlob();

	shaderByteCode->BytecodeLength = blob->GetBufferSize();
	shaderByteCode->pShaderBytecode = blob->GetBufferPointer();
}