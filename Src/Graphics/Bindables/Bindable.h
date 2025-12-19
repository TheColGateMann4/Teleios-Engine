#pragma once
#include "Shaders/TargetShaders.h"

class Graphics;
class CommandList;
class RootSignature;
class GraphicsPipelineState;
class ComputePipelineState;


struct TargetSlotAndShader
{
	ShaderVisibilityGraphic target;
	UINT slot;
	UINT rootIndex = 0;
};


class Bindable
{
public:
	Bindable() = default;
	Bindable(Bindable&&) noexcept = default;
	Bindable& operator=(Bindable&&) noexcept = default;

	virtual ~Bindable() = default;
};



class CommandListBindable
{
public:
	virtual ~CommandListBindable() = default;

	virtual void BindToCommandList(Graphics& graphics, CommandList* commandList) = 0;

	virtual void BindToComputeCommandList(Graphics& graphics, CommandList* commandList);
};

class DirectCommandListBindable
{
public:
	virtual ~DirectCommandListBindable() = default;

	virtual void BindToDirectCommandList(Graphics& graphics, CommandList* commandList) = 0;
};

class RootSignatureBindable
{
public:
	RootSignatureBindable(std::vector<TargetSlotAndShader> targets);

public:
	virtual ~RootSignatureBindable() = default;

	void InternalInitialize(Graphics& graphics);


	virtual void BindToRootSignature(Graphics& graphics, RootSignature* rootSignature) = 0;

	virtual void BindToComputeRootSignature(Graphics& graphics, RootSignature* rootSignature);

	virtual D3D12_GPU_VIRTUAL_ADDRESS GetGPUAddress(Graphics& graphics) const;

	virtual D3D12_GPU_DESCRIPTOR_HANDLE GetDescriptorHeapGPUHandle(Graphics& graphics) const;

	std::vector<TargetSlotAndShader>& GetTargets();

protected:
	virtual void Initialize(Graphics& graphics);

private:
	std::vector<TargetSlotAndShader> m_targets;
	bool m_initialized = false;
};

class PipelineStateBindable
{
public:
	virtual ~PipelineStateBindable() = default;

	virtual void BindToPipelineState(Graphics& graphics, GraphicsPipelineState* pipelineState) = 0;

	virtual void BindToComputePipelineState(Graphics& graphics, ComputePipelineState* pipelineState);
};