#pragma once
#include "Bindable.h"
#include "Graphics.h"

class RootSignature;
class CommandList;

class ShaderResourceView : public Bindable, public CommandListBindable, public RootSignatureBindable
{
public:
	ShaderResourceView(Graphics& graphics, Buffer* buffer, UINT slot = 0);
	ShaderResourceView(ShaderResourceView&&) noexcept = default;

public:
	virtual void BindToCommandList(Graphics& graphics, CommandList* commandList) override;

	virtual void BindToComputeCommandList(Graphics& graphics, CommandList* commandList) override;

	virtual void BindToRootSignature(Graphics& graphics, RootSignature* rootSignature) override;

	virtual void BindToComputeRootSignature(Graphics& graphics, RootSignature* rootSignature) override;

	virtual D3D12_GPU_DESCRIPTOR_HANDLE GetDescriptorHeapGPUHandle(Graphics& graphics) const override;

	void SetComputeRootIndex(unsigned int rootIndex);

	unsigned int GetComputeRootIndex() const;

	unsigned int GetOffsetInDescriptor() const;

private:
	DescriptorHeap::DescriptorInfo m_descriptor = {};
	unsigned int m_computeRootIndex = {};
};