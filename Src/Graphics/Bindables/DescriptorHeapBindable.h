#pragma once
#include "Includes/CppIncludes.h"

#include "Bindable.h"

class Graphics;

class DescriptorHeapBindable : public Bindable, public RootSignatureBindable, public DescriptorBindable
{
public:
	DescriptorHeapBindable(ResourceTargets targets = ResourceTargets{ {ShaderVisibilityGraphic::PixelShader, 0}});

	static std::shared_ptr<DescriptorHeapBindable> GetResource(ResourceTargets targets = ResourceTargets{ {ShaderVisibilityGraphic::PixelShader, 0} });

public:
	static std::string GetIdentifier(ResourceTargets targets);

	virtual void AddGraphicsRootSignatureParam(RootSignatureParams* rootSignatureParams) override;

	virtual void BindToCommandListAsRootParam(Graphics& graphics, CommandList* commandList, const RootBinding& binding) override;

	virtual BindableType GetBindableType() const override;

	virtual RootSignatureBindableType GetRootSignatureBindableType() const override;

	virtual D3D12_GPU_DESCRIPTOR_HANDLE GetDescriptorHeapGPUHandle(Graphics& graphics) const override;

	virtual D3D12_GPU_VIRTUAL_ADDRESS GetGPUAddress(Graphics& graphics) const override;

	virtual void Initialize(Graphics& graphics, DescriptorHeap::DescriptorInfo descriptorInfo, unsigned int descriptorNum) override;

	virtual void Initialize(Graphics& graphics) override;

	virtual DescriptorType GetDescriptorType() const override;

private:
	D3D12_GPU_DESCRIPTOR_HANDLE m_descriptorHandle;
};