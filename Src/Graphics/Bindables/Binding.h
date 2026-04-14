#pragma once
#include "Includes/CppIncludes.h"
#include "Bindable.h"

class CommandListBinding
{
public:
	virtual ~CommandListBinding() = default;

	virtual void BindToCommandList(Graphics& graphics, CommandList* commandList, TargetSlotAndShader& target) = 0;

	virtual void BindToComputeCommandList(Graphics& graphics, CommandList* commandList, TargetSlotAndShader& target);
};

class RootSignatureBinding
{
public:
	RootSignatureBinding(ResourceTargets targets);

public:
	virtual ~RootSignatureBinding() = default;

	virtual void BindToRootSignature(RootSignatureParams* rootSignatureParams, TargetSlotAndShader& target) = 0;

	virtual void AddComputeRootSignatureParam(RootSignatureParams* rootSignatureParams, TargetSlotAndShader& target);

	virtual D3D12_GPU_VIRTUAL_ADDRESS GetGPUAddress(Graphics& graphics) const;

	virtual D3D12_GPU_DESCRIPTOR_HANDLE GetDescriptorHeapGPUHandle(Graphics& graphics) const;

	ResourceTargets& GetTargets();

	virtual RootSignatureBindableType GetRootSignatureBindableType() const = 0;

protected:
	virtual void Initialize(Graphics& graphics);

private:
	ResourceTargets m_targets;
	bool m_initialized = false;
};

class RootParameterBinding : public RootSignatureBinding, public CommandListBinding
{
public:
	RootParameterBinding(ResourceTargets targets);
};