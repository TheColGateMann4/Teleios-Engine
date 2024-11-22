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
enum class ShaderType;

class PipelineState
{
public:
	PipelineState();

public:
	void SetRootSignature(RootSignature* rootSignature);
	void SetShader(Shader* shader);
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
	static D3D12_SHADER_BYTECODE* GetShaderPointerValue(D3D12_GRAPHICS_PIPELINE_STATE_DESC& desc, ShaderType type);

private:
	Microsoft::WRL::ComPtr<ID3D12PipelineState> pPipelineState;
	bool m_finished = false;

	D3D12_GRAPHICS_PIPELINE_STATE_DESC m_desc = {};
};

