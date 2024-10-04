#pragma once
#include "includes/DirectXIncludes.h"
#include "includes/WRLNoWarnings.h"

class PipelineState
{
public:
	PipelineState();

public:
	void SetPixelShader(class Shader* pixelShader);
	void SetVertexShader(class Shader* vertexShader);

public:
	void Finish(class Graphics& graphics);

	ID3D12PipelineState* Get() const;

private:
	static void SetShaderData(Shader* shader, D3D12_SHADER_BYTECODE* shaderByteCode);

private:
	Microsoft::WRL::ComPtr<ID3D12PipelineState> pPipelineState;
	bool m_finished = false;

	D3D12_GRAPHICS_PIPELINE_STATE_DESC m_desc = {};
};

