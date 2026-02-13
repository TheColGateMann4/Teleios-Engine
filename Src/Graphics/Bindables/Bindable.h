#pragma once
#include "Shaders/TargetShaders.h"
#include "BindableTypes.h"

#include "Graphics/Core/DescriptorHeap.h"

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
	virtual BindableType GetBindableType() const = 0;

	virtual ~Bindable() = default;
};

class CommandListBindable
{
public:
	virtual ~CommandListBindable() = default;

	virtual void BindToCommandList(Graphics& graphics, CommandList* commandList) = 0;

	virtual void BindToComputeCommandList(Graphics& graphics, CommandList* commandList);
};

class PipelineStateBindable
{
public:
	virtual ~PipelineStateBindable() = default;

	virtual void BindToPipelineState(Graphics& graphics, GraphicsPipelineState* pipelineState) = 0;

	virtual void BindToComputePipelineState(Graphics& graphics, ComputePipelineState* pipelineState);
};

class RootSignatureBindable
{
public:
	RootSignatureBindable(std::vector<TargetSlotAndShader> targets);

public:
	virtual void BindToRootSignature(RootSignature* rootSignature) = 0;

	virtual void BindToComputeRootSignature(RootSignature* rootSignature);

	std::vector<TargetSlotAndShader>& GetTargets();

private:
	std::vector<TargetSlotAndShader> m_targets;
};


enum class DescriptorType
{
	descriptor_none = -1,
	descriptor_SRV,
	descriptor_UAV,
	descriptor_CBV,
	descriptor_SAMPLER,
};

class DescriptorBindable
{
public:
	virtual void Initialize(Graphics& graphics, DescriptorHeap::DescriptorInfo descriptorInfo, unsigned int descriptorNum) = 0;

	virtual void Initialize(Graphics& graphics) = 0;

public:
	virtual DescriptorType GetDescriptorType() const = 0;
};