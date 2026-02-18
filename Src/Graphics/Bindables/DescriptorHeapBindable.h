#pragma once
#include "Includes/CppIncludes.h"

#include "Bindable.h"

class Graphics;

class DescriptorHeapBindable : public Bindable, public RootSignatureBindable, public CommandListBindable, public DescriptorBindable
{
public:
	DescriptorHeapBindable(std::vector<TargetSlotAndShader> targets = std::vector<TargetSlotAndShader>{ {ShaderVisibilityGraphic::PixelShader, 0}});

public:
	virtual void BindToRootSignature(RootSignature* rootSignature) override;

	virtual void BindToCommandList(Graphics& graphics, CommandList* commandList) override;

	virtual BindableType GetBindableType() const override;

	virtual RootSignatureBindableType GetRootSignatureBindableType() const override;

	D3D12_GPU_DESCRIPTOR_HANDLE GetDescriptorHeapGPUHandle() const;

	virtual void Initialize(Graphics& graphics, DescriptorHeap::DescriptorInfo descriptorInfo, unsigned int descriptorNum) override;

	virtual void Initialize(Graphics& graphics) override;

	virtual DescriptorType GetDescriptorType() const override;

private:
	D3D12_GPU_DESCRIPTOR_HANDLE m_descriptorHandle;
};