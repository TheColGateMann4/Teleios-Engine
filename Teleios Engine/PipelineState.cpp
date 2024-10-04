#include "PipelineState.h"
#include "Macros/ErrorMacros.h"
#include "Graphics.h"
#include "Shader.h"

PipelineState::PipelineState()
{

}

void PipelineState::SetPixelShader(Shader* pixelShader)
{
	SetShaderData(pixelShader, &m_desc.PS);
}

void PipelineState::SetVertexShader(Shader* vertexShader)
{
	SetShaderData(vertexShader, &m_desc.VS);
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