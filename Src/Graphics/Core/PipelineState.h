#pragma once
#include "Includes/DirectXIncludes.h"
#include "Includes/WRLNoWarnings.h"

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
	virtual ~PipelineState() = default;

public:
	virtual void Finish(Graphics& graphics) = 0;

	ID3D12PipelineState* Get() const;

protected:
	Microsoft::WRL::ComPtr<ID3D12PipelineState> pPipelineState;
	bool m_finished = false;
};

class GraphicsPipelineState : public PipelineState
{
public:
	virtual ~GraphicsPipelineState() = default;

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

	virtual void Finish(Graphics& graphics) override;

private:
	static D3D12_SHADER_BYTECODE* GetShaderPointerValue(D3D12_GRAPHICS_PIPELINE_STATE_DESC& desc, ShaderType type);

private:
	D3D12_GRAPHICS_PIPELINE_STATE_DESC m_desc = {};
};

class ComputePipelineState : public PipelineState
{
public:
	virtual ~ComputePipelineState() = default;

public:
	void SetRootSignature(RootSignature* rootSignature);
	void SetShader(Shader* shader);
	// NodeMask;
	// CachedPSO;
	// Flags;

public:
	virtual void Finish(Graphics& graphics) override;

private:
	D3D12_COMPUTE_PIPELINE_STATE_DESC m_desc = {};
};