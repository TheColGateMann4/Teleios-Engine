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
	RootSignatureBinding(std::vector<TargetSlotAndShader> targets);

public:
	virtual ~RootSignatureBinding() = default;

	void InternalInitialize(Graphics& graphics);


	virtual void BindToRootSignature(RootSignature* rootSignature, TargetSlotAndShader& target) = 0;

	virtual void BindToComputeRootSignature(RootSignature* rootSignature, TargetSlotAndShader& target);

	virtual D3D12_GPU_VIRTUAL_ADDRESS GetGPUAddress(Graphics& graphics) const;

	virtual D3D12_GPU_DESCRIPTOR_HANDLE GetDescriptorHeapGPUHandle(Graphics& graphics) const;

	std::vector<TargetSlotAndShader>& GetTargets();

protected:
	virtual void Initialize(Graphics& graphics);

private:
	std::vector<TargetSlotAndShader> m_targets;
	bool m_initialized = false;
};

class RootParameterBinding : public RootSignatureBinding, public CommandListBinding
{
public:
	RootParameterBinding(std::vector<TargetSlotAndShader> targets);
};