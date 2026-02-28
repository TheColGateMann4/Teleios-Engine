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

class GraphicsPipelineStateParams
{
public:
	const D3D12_GRAPHICS_PIPELINE_STATE_DESC* GetDesc() const;

	std::string GetIdentifier() const;

	void Finish();

	bool isFinished() const;

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

private:
	void CreateIdentifier();

private:
	D3D12_GRAPHICS_PIPELINE_STATE_DESC m_desc = {};
	std::vector<Shader*> m_shaders = {};
	RootSignature* m_rootSignature = nullptr;

	std::string m_cachedIdentifier = {};

	bool m_finished = false;
};

class ComputePipelineStateParams
{
public:
	const D3D12_COMPUTE_PIPELINE_STATE_DESC* GetDesc() const;

	std::string GetIdentifier() const;

	void Finish();

	bool isFinished() const;

public:
	void SetRootSignature(RootSignature* rootSignature);
	void SetShader(Shader* shader);
	// NodeMask;
	// CachedPSO;
	// Flags;

private:
	void CreateIdentifier();

private:
	D3D12_COMPUTE_PIPELINE_STATE_DESC m_desc = {};
	Shader* m_computeShader;
	RootSignature* m_rootSignature = nullptr;

	std::string m_cachedIdentifier = {};

	bool m_finished = false;
};

class PipelineState
{
public:
	virtual ~PipelineState() = default;

	ID3D12PipelineState* Get() const;

protected:
	Microsoft::WRL::ComPtr<ID3D12PipelineState> pPipelineState;
};

class GraphicsPipelineState : public PipelineState
{
public:
	GraphicsPipelineState(Graphics& graphics, GraphicsPipelineStateParams&& params);
	virtual ~GraphicsPipelineState() = default;

	static std::shared_ptr<GraphicsPipelineState> GetResource(Graphics& graphics, GraphicsPipelineStateParams&& params);

public:
	static std::string GetIdentifier(const GraphicsPipelineStateParams& params);

private:
	GraphicsPipelineStateParams m_params;
};

class ComputePipelineState : public PipelineState
{
public:
	ComputePipelineState(Graphics& graphics, ComputePipelineStateParams&& params);
	virtual ~ComputePipelineState() = default;

	static std::shared_ptr<ComputePipelineState> GetResource(Graphics& graphics, ComputePipelineStateParams&& params);

public:
	static std::string GetIdentifier(const ComputePipelineStateParams& params);

private:
	ComputePipelineStateParams m_params;
};