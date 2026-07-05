#pragma once
#include "Includes/CppIncludes.h"
#include "Includes/DirectXIncludes.h"
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
};

struct RootBinding
{
	TargetSlotAndShader target;
	UINT rootIndex = 0;
};

using ResourceTargets = std::vector<TargetSlotAndShader>;

class Bindable
{
public:
	Bindable() = default;
	Bindable(Bindable&&) noexcept = default;
	Bindable& operator=(Bindable&&) noexcept = default;
	virtual BindableType GetBindableType() const = 0;

	virtual ~Bindable() = default;
};

class UpdatableBindable
{
public:
	void SetUpdated();
	unsigned long long GetRevision() const;

private:
	unsigned long long m_revision = 0;
};

class CommandListBindable : public virtual UpdatableBindable
{
public:
	virtual ~CommandListBindable() = default;

	virtual void BindToCommandList(Graphics& graphics, CommandList* commandList) = 0;

	virtual void BindToComputeCommandList(Graphics& graphics, CommandList* commandList);
};

class PipelineStateBindable : public virtual UpdatableBindable
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

class RootSignatureBindable : public virtual UpdatableBindable
{
public:
	RootSignatureBindable(ResourceTargets targets);

public:
	virtual void AddGraphicsRootSignatureParam(RootSignatureParams* rootSignatureParams) = 0;

	virtual void BindToCommandListAsRootParam(Graphics& graphics, CommandList* commandList, const RootBinding& binding) = 0;

	const ResourceTargets& GetTargets() const;

	virtual RootSignatureBindableType GetRootSignatureBindableType() const = 0;

	virtual D3D12_GPU_VIRTUAL_ADDRESS GetGPUAddress(Graphics& graphics) const = 0;

private:
	ResourceTargets m_targets;
};


enum class DescriptorType
{
	descriptor_none = -1,
	descriptor_SRV,
	descriptor_UAV,
	descriptor_CBV,
	descriptor_SAMPLER,
};

class DescriptorBindable : public virtual UpdatableBindable
{
public:
	virtual void Initialize(Graphics& graphics, DescriptorHeap::DescriptorInfo descriptorInfo, unsigned int descriptorNum) = 0;

	virtual void Initialize(Graphics& graphics) = 0;

	virtual D3D12_GPU_DESCRIPTOR_HANDLE GetDescriptorHeapGPUHandle(Graphics& graphics) const = 0;

public:
	virtual DescriptorType GetDescriptorType() const = 0;
};