#pragma once
#include "Includes/CppIncludes.h"

#include "Bindable.h"

class Graphics;

class DescriptorHeapBindable : public Bindable, public RootSignatureBindable, public CommandListBindable
{
public:
	DescriptorHeapBindable(Graphics& graphics, std::vector<TargetSlotAndShader> targets = std::vector<TargetSlotAndShader>{ {ShaderVisibilityGraphic::PixelShader, 0}});

public:
	virtual void BindToRootSignature(RootSignature* rootSignature) override;

	virtual void BindToCommandList(Graphics& graphics, CommandList* commandList) override;

	D3D12_GPU_DESCRIPTOR_HANDLE GetDescriptorHeapGPUHandle() const;

private:
	D3D12_GPU_DESCRIPTOR_HANDLE m_descriptorHandle;
};