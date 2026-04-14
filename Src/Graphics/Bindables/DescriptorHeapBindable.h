#pragma once
#include "Includes/CppIncludes.h"

#include "Bindable.h"
#include "Binding.h"

class Graphics;

class DescriptorHeapBindable : public Bindable, public RootParameterBinding, public DescriptorBindable
{
public:
	DescriptorHeapBindable(ResourceTargets targets = ResourceTargets{ {ShaderVisibilityGraphic::PixelShader, 0}});

	static std::shared_ptr<DescriptorHeapBindable> GetResource(ResourceTargets targets = ResourceTargets{ {ShaderVisibilityGraphic::PixelShader, 0} });

public:
	static std::string GetIdentifier(ResourceTargets targets);

	virtual void BindToRootSignature(RootSignatureParams* rootSignatureParams, TargetSlotAndShader& target) override;

	virtual void BindToCommandList(Graphics& graphics, CommandList* commandList, TargetSlotAndShader& target) override;

	virtual BindableType GetBindableType() const override;

	virtual RootSignatureBindableType GetRootSignatureBindableType() const override;

	D3D12_GPU_DESCRIPTOR_HANDLE GetDescriptorHeapGPUHandle() const;

	virtual void Initialize(Graphics& graphics, DescriptorHeap::DescriptorInfo descriptorInfo, unsigned int descriptorNum) override;

	virtual void Initialize(Graphics& graphics) override;

	virtual DescriptorType GetDescriptorType() const override;

private:
	D3D12_GPU_DESCRIPTOR_HANDLE m_descriptorHandle;
};