#pragma once
#include "includes/DirectXIncludes.h"
#include "includes/WRLNoWarnings.h"

class Graphics;

class RootSignature;
class Shader;
class BlendState;
class RasterizerState;
class DepthStencilState;
class InputLayout;

class PipelineState
{
public:
	PipelineState();

public:
	void SetRootSignature(RootSignature* rootSignature);
	void SetVertexShader(Shader* vertexShader);
	void SetPixelShader(Shader* pixelShader);
	// DS
	// HS
	// GS
	// StreamOutput
	// IBStripCutValue
	void SetBlendState(BlendState* blendState);
	void SetSampleMask(UINT sampleMask);
	void SetRasterizerState(RasterizerState* rasterizerState);
	void SetDepthStencilState(DepthStencilState* depthStencilState);
	void SetInputLayout(InputLayout* inputLayout);
	void SetPrimitiveTechnologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE technologyType);
	void SetNumRenderTargets(UINT numRenderTargets);
	void SetRenderTargetFormat(UINT index, DXGI_FORMAT renderTargetFormat);
	void SetDepthStencilFormat(DXGI_FORMAT depthStencilFormat);
	void SetSampleDesc(UINT count, UINT quality);
	// NodeMask
	// CachedPSO
	// Flags

public:
	void Finish(Graphics& graphics);

	ID3D12PipelineState* Get() const;

private:
	static void SetShaderData(Shader* shader, D3D12_SHADER_BYTECODE* shaderByteCode);

private:
	Microsoft::WRL::ComPtr<ID3D12PipelineState> pPipelineState;
	bool m_finished = false;

	D3D12_GRAPHICS_PIPELINE_STATE_DESC m_desc = {};
};

