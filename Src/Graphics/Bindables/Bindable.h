#pragma once
#include "Shaders/TargetShaders.h"
#include "BindableTypes.h"

#include "Graphics/Core/DescriptorHeap.h"

class Graphics;
class CommandList;
class RootSignatureParams;
class GraphicsPipelineStateParams;
class ComputePipelineStateParams;


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

	virtual void AddPipelineStateParam(Graphics& graphics, GraphicsPipelineStateParams* params) = 0;

	virtual void AddComputePipelineStateParam(Graphics& graphics, ComputePipelineStateParams* params);
};

enum class RootSignatureBindableType
{
	rootSignature_none = -1,
	rootSignature_DescriptorTable,
	rootSignature_Constants,
	rootSignature_CBV,
	rootSignature_BufferSRV,
	rootSignature_BufferUAV,
	rootSignature_StaticSampler,
};

class RootSignatureBindable
{
public:
	RootSignatureBindable(std::vector<TargetSlotAndShader> targets);

public:
	virtual void BindToRootSignature(RootSignatureParams* rootSignatureParams) = 0;

	virtual void AddComputeRootSignatureParam(RootSignatureParams* rootSignatureParams);

	std::vector<TargetSlotAndShader>& GetTargets();

	virtual RootSignatureBindableType GetRootSignatureBindableType() const = 0;

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